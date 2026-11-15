#include "LoopSessionSubsystem.h"

#include "Core/LoopSaveGame.h"
#include "Core/MyPlayerController.h"
#include "Rendering/LoopWorldColorSubsystem.h"
#include "Characters/PlayerCharacter.h"
#include "Actors/GroundBlock.h"
#include "Actors/LiftRegionController.h"
#include "Actors/PressurePlateBase.h"
#include "Actors/WorldsBasicActor.h"
#include "Actors/TutorialOpeningSequence.h"
#include "Actors/MainRingSequence.h"
#include "Settings/LoopSettings.h"
#include "UI/LoopMainMenuWidget.h"
#include "UI/LoopPauseMenuWidget.h"
#include "UI/LoopNarrationDirector.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameModeBase.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/Font.h"
#include "HAL/PlatformTime.h"
#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "TimerManager.h"

namespace
{
	const FName TutorialMap(TEXT("/Game/MegaJam2026/Maps/Gameplay/L_Tutorial"));
	const FName MainMap(TEXT("/Game/MegaJam2026/Maps/Gameplay/L_Main"));
	bool IsSupportedMap(FName Map)
	{
		return Map == TutorialMap || Map == FName(TEXT("/Game/MegaJam2026/Maps/Gameplay/L_Main"));
	}
	FName CurrentMap(const UObject* Context)
	{
		return FName(*(TEXT("/Game/MegaJam2026/Maps/Gameplay/") + UGameplayStatics::GetCurrentLevelName(Context, true)));
	}
	bool IsProgressActor(AActor* Actor)
	{
		return Actor->IsA<AGroundBlock>() || Actor->IsA<AWorldsBasicActor>() || Actor->IsA<ALiftRegionController>()
			|| Actor->IsA<ATutorialOpeningSequence>()
			|| Actor->IsA<AMainRingSequence>()
			|| Actor->IsA<ALoopNarrationDirector>()
			|| Actor->ActorHasTag(TEXT("Refunct_TouchSurface"));
	}
	bool IsEndingPlaying(UWorld* World)
	{
		for (TActorIterator<AMainRingSequence> Sequence(World); Sequence; ++Sequence)
			if (Sequence->IsEndingPlaying()) return true;
		return false;
	}
}

void ULoopSessionSubsystem::InitializeController(AMyPlayerController* PlayerController)
{
	Controller = PlayerController;
	if (IntroPhase == EIntroPhase::Loading)
	{
		BeginTutorialReveal();
		return;
	}
	bTransitioning = false;
	if (PendingSave)
	{
		bAtMainMenu = false;
		ApplyPendingSave();
		return;
	}
	if (bMenuOnNextLoad && CurrentMap(PlayerController) == TutorialMap)
	{
		ShowOpeningMenu();
		return;
	}
	bAtMainMenu = false;
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->SetShowMouseCursor(false);
}

void ULoopSessionSubsystem::ShowOpeningMenu()
{
	AMyPlayerController* PlayerController = Controller.Get();
	if (!PlayerController || !PlayerController->MainMenuClass)
	{
		return;
	}
	bAtMainMenu = true;
	SetOpeningSceneColor(true);
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerController->GetPawn()))
	{
		Player->PrepareForMenu();
		Player->GetCharacterMovement()->DisableMovement();
	}
	MainMenu = ULoopMainMenuWidget::ShowMainMenu(PlayerController, PlayerController->MainMenuClass);
}

void ULoopSessionSubsystem::SetOpeningSceneColor(bool bVisible)
{
	if (!Controller.IsValid()) return;
	for (TActorIterator<AGroundBlock> Block(Controller->GetWorld()); Block; ++Block)
	{
		if (Block->ActorHasTag(TEXT("OpeningMenuColor"))) Block->SetMenuColorVisible(bVisible);
	}
}

bool ULoopSessionSubsystem::HasSavedGame() const
{
	const ULoopSaveGame* Save = Cast<ULoopSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	return Save && Save->Version == 1 && IsSupportedMap(Save->Map)
		&& !Save->PlayerTransform.ContainsNaN() && !Save->Gravity.IsNearlyZero() && !Save->Gravity.ContainsNaN();
}

void ULoopSessionSubsystem::PrepareTravel()
{
	if (IntroPhase == EIntroPhase::AwaitLanding) ClearTutorialTransition();
	SetOpeningSceneColor(false);
	bTransitioning = true;
	if (AMyPlayerController* PlayerController = Controller.Get())
	{
		PlayerController->SetPause(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}
	if (PauseMenu) PauseMenu->RemoveFromParent();
	if (MainMenu) MainMenu->RemoveFromParent();
	PauseMenu = nullptr;
	MainMenu = nullptr;
	bAtMainMenu = false;
}

void ULoopSessionSubsystem::NewGame()
{
	if (bTransitioning || !Controller.IsValid()) return;
	bMainChapterTransition = false;
	BeginChapterTransition();
}

void ULoopSessionSubsystem::StartMainChapter()
{
	if (bTransitioning || !Controller.IsValid() || bAtMainMenu || CurrentMap(Controller.Get()) != TutorialMap) return;
	bMainChapterTransition = true;
	BeginChapterTransition();
}

void ULoopSessionSubsystem::BeginChapterTransition()
{
	ClearTutorialTransition();
	for (TActorIterator<ALoopNarrationDirector> Director(Controller->GetWorld()); Director; ++Director)
	{
		Director->StopNarration();
	}
	PendingSave = nullptr;
	bMenuOnNextLoad = false;
	bTransitioning = true;
	IntroPhase = EIntroPhase::FadeOut;
	IntroElapsed = 0.0f;
	IntroPhaseStartedAt = FPlatformTime::Seconds();
	IntroFadeOpacity = 0.0f;
	bIntroNarrationStarted = false;
	IntroViewport = GetGameInstance()->GetGameViewportClient();
	if (!IntroViewport.IsValid())
	{
		ClearTutorialTransition();
		bTransitioning = false;
		return;
	}
	IntroTitleFont = LoadObject<UFont>(nullptr, TEXT("/Game/MegaJam2026/UI/Fonts/NotoSansCJKsc_Font.NotoSansCJKsc_Font"));
	IntroOverlay = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor_Lambda([this]() { return FSlateColor(FLinearColor(0, 0, 0, IntroFadeOpacity)); })
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { return GetIntroTitle(); })
			.Font(FSlateFontInfo(IntroTitleFont, 42, FName(TEXT("Default"))))
			.Justification(ETextJustify::Center)
			.Visibility_Lambda([this]() { return IntroPhase == EIntroPhase::Hold ? EVisibility::HitTestInvisible : EVisibility::Hidden; })
			.ColorAndOpacity_Lambda([this]()
			{
				const float TitleOpacity = FMath::Min(FMath::Clamp(IntroElapsed / 0.35f, 0.0f, 1.0f),
					FMath::Clamp((3.0f - IntroElapsed) / 0.35f, 0.0f, 1.0f));
				return FSlateColor(FLinearColor(1, 1, 1, TitleOpacity));
			})
		];
	IntroViewport->AddViewportWidgetContent(IntroOverlay.ToSharedRef(), 10000);
	Controller->SetShowMouseCursor(false);
	if (bMainChapterTransition)
	{
		Controller->SetIgnoreMoveInput(true);
		Controller->SetIgnoreLookInput(true);
	}
	if (MainMenu) MainMenu->SetIsEnabled(false);
	IntroTicker = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &ULoopSessionSubsystem::TickTutorialIntro));
}

void ULoopSessionSubsystem::BeginTutorialReveal()
{
	AMyPlayerController* PlayerController = Controller.Get();
	APlayerCharacter* Player = PlayerController ? Cast<APlayerCharacter>(PlayerController->GetPawn()) : nullptr;
	if (!Player)
	{
		ClearTutorialTransition();
		bTransitioning = false;
		return;
	}
	Player->PrepareForMenu();
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
	Player->GetCharacterMovement()->DisableMovement();
	if (AGameModeBase* GameMode = UGameplayStatics::GetGameMode(Player))
	{
		if (AActor* Start = GameMode->FindPlayerStart(PlayerController))
		{
			Player->SetActorLocationAndRotation(Start->GetActorLocation(), Start->GetActorRotation(), false, nullptr, ETeleportType::TeleportPhysics);
			PlayerController->SetControlRotation(Start->GetActorRotation());
		}
	}
	PlayerController->SetViewTarget(Player);
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetIgnoreMoveInput(true);
	PlayerController->SetIgnoreLookInput(true);
	IntroFadeOpacity = 1.0f;
	IntroElapsed = 0.0f;
	IntroPhase = EIntroPhase::Hold;
	IntroPhaseStartedAt = FPlatformTime::Seconds();
}

bool ULoopSessionSubsystem::TickTutorialIntro(float DeltaSeconds)
{
	IntroElapsed = static_cast<float>(FPlatformTime::Seconds() - IntroPhaseStartedAt);
	if (IntroPhase == EIntroPhase::FadeOut)
	{
		IntroFadeOpacity = FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp(IntroElapsed / 0.55f, 0.0f, 1.0f));
		if (IntroElapsed >= 0.55f)
		{
			IntroPhase = EIntroPhase::Blackout;
			IntroPhaseStartedAt = FPlatformTime::Seconds();
		}
	}
	else if (IntroPhase == EIntroPhase::Blackout && IntroElapsed >= 0.1f)
	{
		IntroPhase = EIntroPhase::Loading;
		PrepareTravel();
		UGameplayStatics::OpenLevel(Controller.Get(), bMainChapterTransition ? MainMap : TutorialMap);
	}
	else if (IntroPhase == EIntroPhase::Hold && IntroElapsed >= 3.0f)
	{
		if (AMyPlayerController* PlayerController = Controller.Get())
		{
			if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerController->GetPawn()))
			{
				Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			}
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);
		}
		IntroPhase = EIntroPhase::Reveal;
		IntroElapsed = 0.0f;
		IntroPhaseStartedAt = FPlatformTime::Seconds();
		IntroOverlay->SetVisibility(EVisibility::HitTestInvisible);
	}
	else if (IntroPhase == EIntroPhase::Reveal)
	{
		IntroFadeOpacity = 1.0f - FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp(IntroElapsed / 1.5f, 0.0f, 1.0f));
		if (IntroElapsed >= 1.5f)
		{
			IntroFadeOpacity = 0.0f;
			if (IntroViewport.IsValid() && IntroOverlay.IsValid())
			{
				IntroViewport->RemoveViewportWidgetContent(IntroOverlay.ToSharedRef());
			}
			IntroOverlay.Reset();
			IntroViewport.Reset();
			IntroPhase = EIntroPhase::AwaitLanding;
			bTransitioning = false;
		}
	}
	if (IntroPhase == EIntroPhase::AwaitLanding && Controller.IsValid() && !UGameplayStatics::IsGamePaused(Controller.Get()))
	{
		const ACharacter* Player = Cast<ACharacter>(Controller->GetPawn());
		if (Player && Player->GetCharacterMovement()->IsMovingOnGround())
		{
			bIntroNarrationStarted = true;
			PlayTutorialNarration();
			ClearTutorialTransition();
			return false;
		}
	}
	return IntroPhase != EIntroPhase::None;
}

FText ULoopSessionSubsystem::GetIntroTitle() const
{
	if (bMainChapterTransition)
	{
		return ULoopSettings::GetLoopSettings()->GameplayLanguage == ELoopGameplayLanguage::Chinese
			? FText::FromString(TEXT("第二幕：“∞”"))
			: FText::FromString(TEXT("Act II: \"∞\""));
	}
	return ULoopSettings::GetLoopSettings()->GameplayLanguage == ELoopGameplayLanguage::Chinese
		? FText::FromString(TEXT("第一幕：“教学”"))
		: FText::FromString(TEXT("Act I: \"Tutorial\""));
}

void ULoopSessionSubsystem::PlayTutorialNarration()
{
	if (!Controller.IsValid()) return;
	for (TActorIterator<ALoopNarrationDirector> Director(Controller->GetWorld()); Director; ++Director)
	{
		if (Director->Script)
		{
			Director->PlaySection(bMainChapterTransition ? TEXT("Main.Landing") : TEXT("Tutorial.Intro"));
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Tutorial intro requires a LoopNarrationDirector with a narration table."));
}

void ULoopSessionSubsystem::ClearTutorialTransition()
{
	FTSTicker::GetCoreTicker().RemoveTicker(IntroTicker);
	IntroTicker.Reset();
	if (IntroViewport.IsValid() && IntroOverlay.IsValid())
	{
		IntroViewport->RemoveViewportWidgetContent(IntroOverlay.ToSharedRef());
	}
	IntroOverlay.Reset();
	IntroTitleFont = nullptr;
	IntroViewport.Reset();
	IntroPhase = EIntroPhase::None;
	IntroFadeOpacity = 0.0f;
}

void ULoopSessionSubsystem::Deinitialize()
{
	ClearTutorialTransition();
	Super::Deinitialize();
}

bool ULoopSessionSubsystem::ContinueGame()
{
	if (bTransitioning || !Controller.IsValid() || !HasSavedGame()) return false;
	PendingSave = Cast<ULoopSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	bMenuOnNextLoad = false;
	const FName Map = PendingSave->Map;
	PrepareTravel();
	UGameplayStatics::OpenLevel(Controller.Get(), Map);
	return true;
}

bool ULoopSessionSubsystem::SaveCurrentGame()
{
	AMyPlayerController* PlayerController = Controller.Get();
	APlayerCharacter* Player = PlayerController ? Cast<APlayerCharacter>(PlayerController->GetPawn()) : nullptr;
	if (!Player || bAtMainMenu || bTransitioning || !IsSupportedMap(CurrentMap(Player)) || IsEndingPlaying(Player->GetWorld())) return false;
	ULoopSaveGame* Save = Cast<ULoopSaveGame>(UGameplayStatics::CreateSaveGameObject(ULoopSaveGame::StaticClass()));
	Save->Map = CurrentMap(Player);
	Save->PlayerTransform = Player->GetActorTransform();
	Save->ControlRotation = PlayerController->GetControlRotation();
	Save->Gravity = Player->GetCharacterMovement()->GetGravityDirection();
	Save->Velocity = Player->GetCharacterMovement()->Velocity;
	Save->MovementMode = Player->GetCharacterMovement()->MovementMode;
	Save->bCrouched = Player->bIsCrouched;
	if (const ULoopWorldColorSubsystem* WorldColor = Player->GetWorld()->GetSubsystem<ULoopWorldColorSubsystem>())
	{
		Save->bWorldColorRestored = WorldColor->IsWorldColorRestored();
	}
	Save->SpawnPoint = Player->GetSavedSpawnPoint();
	for (TActorIterator<AActor> Actor(Player->GetWorld()); Actor; ++Actor)
	{
		if (!IsProgressActor(*Actor)) continue;
		if (ALoopNarrationDirector* Director = Cast<ALoopNarrationDirector>(*Actor)) Director->CaptureSavedState();
		FLoopSavedActor& State = Save->Actors.AddDefaulted_GetRef();
		State.Name = Actor->GetFName();
		State.Transform = Actor->GetActorTransform();
		FMemoryWriter Writer(State.Data);
		FObjectAndNameAsStringProxyArchive Archive(Writer, true);
		Archive.ArIsSaveGame = true;
		Actor->Serialize(Archive);
		TInlineComponentArray<UStaticMeshComponent*> Meshes;
		Actor->GetComponents(Meshes);
		for (UStaticMeshComponent* Mesh : Meshes)
		{
			FLoopSavedMesh& MeshState = State.Meshes.AddDefaulted_GetRef();
			MeshState.Component = Mesh->GetFName();
			MeshState.bCustomDepth = Mesh->bRenderCustomDepth;
			MeshState.Stencil = Mesh->CustomDepthStencilValue;
			for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
			{
				FLoopSavedMaterial& MaterialState = MeshState.Materials.AddDefaulted_GetRef();
				UMaterialInterface* Material = Mesh->GetMaterial(Index);
				if (UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(Material))
				{
					TArray<FMaterialParameterInfo> Infos;
					TArray<FGuid> Guids;
					Dynamic->GetAllVectorParameterInfo(Infos, Guids);
					for (const FMaterialParameterInfo& Info : Infos)
					{
						FLinearColor Value;
						if (Dynamic->GetVectorParameterValue(Info, Value)) MaterialState.Vectors.Add(Info.Name, Value);
					}
					Dynamic->GetAllScalarParameterInfo(Infos, Guids);
					for (const FMaterialParameterInfo& Info : Infos)
					{
						float Value = 0;
						if (Dynamic->GetScalarParameterValue(Info, Value)) MaterialState.Scalars.Add(Info.Name, Value);
					}
					Material = Dynamic->Parent;
					while (UMaterialInstanceDynamic* Parent = Cast<UMaterialInstanceDynamic>(Material)) Material = Parent->Parent;
				}
				MaterialState.Material = FSoftObjectPath(Material);
			}
		}
	}
	return UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
}

void ULoopSessionSubsystem::ApplyPendingSave()
{
	AMyPlayerController* PlayerController = Controller.Get();
	APlayerCharacter* Player = PlayerController ? Cast<APlayerCharacter>(PlayerController->GetPawn()) : nullptr;
	if (!Player || !PendingSave) return;
	TMap<FName, AActor*> Actors;
	for (TActorIterator<AActor> Actor(Player->GetWorld()); Actor; ++Actor) Actors.Add(Actor->GetFName(), *Actor);
	for (const FLoopSavedActor& State : PendingSave->Actors)
	{
		AActor* Actor = Actors.FindRef(State.Name);
		if (!Actor || !IsProgressActor(Actor)) continue;
		FMemoryReader Reader(State.Data);
		FObjectAndNameAsStringProxyArchive Archive(Reader, true);
		Archive.ArIsSaveGame = true;
		Actor->Serialize(Archive);
		Actor->SetActorTransform(State.Transform, false, nullptr, ETeleportType::TeleportPhysics);
		if (AGroundBlock* Ground = Cast<AGroundBlock>(Actor)) Ground->RestoreSavedState();
		if (APressurePlateBase* Plate = Cast<APressurePlateBase>(Actor)) Plate->RestoreSavedState();
		if (ALiftRegionController* Region = Cast<ALiftRegionController>(Actor)) Region->RestoreSavedState();
		TInlineComponentArray<UStaticMeshComponent*> Meshes;
		Actor->GetComponents(Meshes);
		for (const FLoopSavedMesh& MeshState : State.Meshes)
		{
			UStaticMeshComponent** Found = Meshes.FindByPredicate([&](UStaticMeshComponent* Mesh) { return Mesh->GetFName() == MeshState.Component; });
			if (!Found) continue;
			UStaticMeshComponent* Mesh = *Found;
			if (Actor->IsA<APressurePlateBase>() && Mesh->GetFName() == FName(TEXT("PlateMesh"))) continue;
			if (Mesh->GetFName() == FName(TEXT("CenterMarker"))) continue;
			const bool bInvertedColor = Actor->ActorHasTag(TEXT("InvertedWorldColor"));
			Mesh->SetRenderCustomDepth(bInvertedColor);
			Mesh->SetCustomDepthStencilValue(bInvertedColor ? 1 : 0);
			for (int32 Index = 0; Index < MeshState.Materials.Num(); ++Index)
			{
				const FLoopSavedMaterial& MaterialState = MeshState.Materials[Index];
				UMaterialInterface* Material = Cast<UMaterialInterface>(MaterialState.Material.TryLoad());
				if (!Material) continue;
				if (MaterialState.Vectors.Num() || MaterialState.Scalars.Num())
				{
					UMaterialInstanceDynamic* Dynamic = Mesh->CreateDynamicMaterialInstance(Index, Material);
					for (const auto& Value : MaterialState.Vectors) Dynamic->SetVectorParameterValue(Value.Key, Value.Value);
					for (const auto& Value : MaterialState.Scalars) Dynamic->SetScalarParameterValue(Value.Key, Value.Value);
				}
				else Mesh->SetMaterial(Index, Material);
			}
		}
	}
	for (TActorIterator<ATutorialOpeningSequence> Sequence(Player->GetWorld()); Sequence; ++Sequence)
	{
		Sequence->RestoreSavedState();
	}
	for (TActorIterator<ALoopNarrationDirector> Director(Player->GetWorld()); Director; ++Director)
	{
		Director->RestoreSavedState();
	}
	for (TActorIterator<AMainRingSequence> Sequence(Player->GetWorld()); Sequence; ++Sequence)
	{
		Sequence->RestoreSavedState();
	}
	Player->RestoreSavedPlayer(*PendingSave);
	if (ULoopWorldColorSubsystem* WorldColor = Player->GetWorld()->GetSubsystem<ULoopWorldColorSubsystem>())
	{
		WorldColor->RestoreSavedState(PendingSave->bWorldColorRestored);
	}
	PendingSave = nullptr;
	PlayerController->SetInputMode(FInputModeGameOnly());
	PlayerController->SetShowMouseCursor(false);
}

void ULoopSessionSubsystem::TogglePauseMenu()
{
	AMyPlayerController* PlayerController = Controller.Get();
	if (!PlayerController || bAtMainMenu || bTransitioning || IsEndingPlaying(PlayerController->GetWorld())) return;
	if (PauseMenu)
	{
		ResumeGame();
		return;
	}
	if (!PlayerController->PauseMenuClass) return;
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerController->GetPawn())) Player->PrepareForMenu();
	PauseMenu = CreateWidget<ULoopPauseMenuWidget>(PlayerController, PlayerController->PauseMenuClass);
	if (!PauseMenu) return;
	PlayerController->SetPause(true);
	PauseMenu->AddToViewport(300);
	PlayerController->SetShowMouseCursor(true);
	FInputModeUIOnly Input;
	Input.SetWidgetToFocus(PauseMenu->TakeWidget());
	PlayerController->SetInputMode(Input);
	PauseMenu->SetKeyboardFocus();
}

void ULoopSessionSubsystem::ResumeGame()
{
	if (!PauseMenu) return;
	PauseMenu->RemoveFromParent();
	PauseMenu = nullptr;
	if (AMyPlayerController* PlayerController = Controller.Get())
	{
		PlayerController->SetPause(false);
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(PlayerController->GetPawn())) Player->PrepareForMenu();
	}
}

bool ULoopSessionSubsystem::SaveAndExit()
{
	if (!SaveCurrentGame()) return false;
	if (bQuitOnSaveExit)
	{
		UKismetSystemLibrary::QuitGame(Controller.Get(), Controller.Get(), EQuitPreference::Quit, false);
		return true;
	}
	bMenuOnNextLoad = true;
	PrepareTravel();
	UGameplayStatics::OpenLevel(Controller.Get(), TutorialMap);
	return true;
}

void ULoopSessionSubsystem::RestartLevel()
{
	if (!Controller.IsValid() || bTransitioning || bAtMainMenu) return;
	const FName Map = CurrentMap(Controller.Get());
	if (!IsSupportedMap(Map)) return;
	PendingSave = nullptr;
	bMenuOnNextLoad = false;
	PrepareTravel();
	UGameplayStatics::OpenLevel(Controller.Get(), Map);
}
