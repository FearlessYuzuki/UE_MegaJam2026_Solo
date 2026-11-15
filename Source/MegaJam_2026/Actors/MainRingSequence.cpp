#include "Actors/MainRingSequence.h"

#include "Actors/GroundBlock.h"
#include "Actors/PressurePlateGravity.h"
#include "Actors/PressurePlateLaunch.h"
#include "Actors/PressurePlateGoal.h"
#include "Characters/PlayerCharacter.h"
#include "Core/LoopSessionSubsystem.h"
#include "UI/LoopNarrationDirector.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "MediaPlayer.h"
#include "MediaSource.h"
#include "UI/LoopEndingWidget.h"
#include "Rendering/LoopWorldColorSubsystem.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

AMainRingSequence::AMainRingSequence()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
}

void AMainRingSequence::BeginPlay()
{
	Super::BeginPlay();
	if (OpeningBlock) CheckpointBlock = OpeningBlock->GetFName();
	if (FinaleLaunch) FinaleLaunch->OnLaunchFired.AddUniqueDynamic(this, &AMainRingSequence::HandleFinaleLaunch);
	if (CompletionPlate) CompletionPlate->OnGoalReached.AddUniqueDynamic(this, &AMainRingSequence::HandleCompletion);
	if (NarrationDirector) NarrationDirector->OnSectionFinished.AddUniqueDynamic(this, &AMainRingSequence::HandleSectionFinished);
	ApplyGateState();
}

void AMainRingSequence::ApplyGateState()
{
	for (int32 StageIndex = 0; StageIndex < Stages.Num(); ++StageIndex)
	{
		if (APressurePlateGravity* Plate = Stages[StageIndex].ExitPlate)
		{
			Plate->SetActorEnableCollision(bCelebrationCompleted || StageIndex == CompletedFlips);
		}
	}
	if (FinaleLaunch) FinaleLaunch->SetActorEnableCollision(!bCelebrationCompleted && CompletedFlips >= Stages.Num());
	if (CompletionPlate)
	{
		CompletionPlate->SetActorHiddenInGame(!bEndingFinished);
		CompletionPlate->SetActorEnableCollision(bEndingFinished);
	}
}

void AMainRingSequence::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsEndingPlaying())
	{
		TickEnding(DeltaSeconds);
		return;
	}
	const ULoopSessionSubsystem* Session = GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>();
	if (Session && (Session->IsAtMainMenu() || Session->IsTutorialIntroPlaying())) return;
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player) return;
	if (bCelebrationCompleted)
	{
		for (const FMainRingStage& Stage : Stages)
			if (Stage.ExitPlate && !Stage.ExitPlate->CanPressAgain() && Stage.ExitPlate->GetOverlapCount() == 0) Stage.ExitPlate->ResetPlate();
		const FVector Gravity = Player->GetCharacterMovement()->GetGravityDirection();
		if (!ExplorationGravity.Equals(Gravity))
		{
			ExplorationGravity = Gravity;
			float ClosestDistance = MAX_flt;
			for (AGroundBlock* Block : CourseBlocks)
			{
				if (!Block || FVector::DotProduct(Block->GetActorUpVector(), -Gravity) < 0.9f) continue;
				const float Distance = FVector::DistSquaredXY(Player->GetActorLocation(), Block->GetActorLocation());
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					CheckpointBlock = Block->GetFName();
				}
			}
		}
		UpdateRecovery();
		return;
	}
	if (!bLandingNarrationStarted && Player->GetCharacterMovement()->IsMovingOnGround())
	{
		bLandingNarrationStarted = true;
		if (NarrationDirector) NarrationDirector->QueueSection(TEXT("Main.Landing"));
	}
	if (Stages.IsValidIndex(CompletedFlips))
	{
		const FMainRingStage& Stage = Stages[CompletedFlips];
		if (Stage.ExitPlate && Stage.ExitPlate->IsPressed())
		{
			++CompletedFlips;
			AGroundBlock* Destination = Stages.IsValidIndex(CompletedFlips) ? Stages[CompletedFlips].LandingBlock.Get() : FinaleBlock.Get();
			if (Destination) CheckpointBlock = Destination->GetFName();
			if (Stages.IsValidIndex(CompletedFlips))
			{
				for (AGroundBlock* Block : Stages[CompletedFlips].RevealBlocks)
				{
					if (Block) Block->StartLift();
				}
			}
			if (NarrationDirector && CompletedFlips <= 3)
			{
				NarrationDirector->QueueSection(FName(*FString::Printf(TEXT("Main.Flip%d"), CompletedFlips)));
			}
			ApplyGateState();
		}
	}
	UpdateRecovery();
}

void AMainRingSequence::UpdateRecovery()
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player) return;
	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	const FVector Gravity = Movement->GetGravityDirection();
	AGroundBlock* Checkpoint = nullptr;
	for (AGroundBlock* Block : CourseBlocks)
	{
		if (!IsValid(Block) || FVector::DotProduct(Block->GetActorUpVector(), -Gravity) < 0.9f) continue;
		if (Block->GetFName() == CheckpointBlock) Checkpoint = Block;
		if (Movement->IsMovingOnGround() && Movement->CurrentFloor.HitResult.GetActor() == Block)
		{
			CheckpointBlock = Block->GetFName();
			return;
		}
	}
	if (!Checkpoint) return;
	const UStaticMeshComponent* Ground = Checkpoint->FindComponentByClass<UStaticMeshComponent>();
	if (!Ground || !Ground->GetStaticMesh()) return;
	const FBox Bounds = Ground->GetStaticMesh()->GetBoundingBox();
	const FVector Surface = Ground->GetComponentTransform().TransformPosition(FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Max.Z));
	const FVector Feet = Player->GetActorLocation() + Gravity * Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	if (FVector::DotProduct(Feet - Surface, Gravity) <= RecoveryFallDistance) return;
	const ACharacter* DefaultPlayer = Player->GetClass()->GetDefaultObject<ACharacter>();
	const float HalfHeight = DefaultPlayer->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * Player->GetCapsuleComponent()->GetShapeScale();
	Player->PrepareForMenu();
	Movement->StopMovementImmediately();
	Movement->SetMovementMode(MOVE_Falling);
	Player->SetActorLocation(Surface - Gravity * (HalfHeight + 5.0f), false, nullptr, ETeleportType::TeleportPhysics);
	Player->UnCrouch();
	if (APlayerController* Controller = Cast<APlayerController>(Player->GetController()))
	{
		if (Controller->PlayerCameraManager) Controller->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 0.3f, FLinearColor::Black);
	}
}

void AMainRingSequence::HandleFinaleLaunch(AActor* LaunchedActor, FVector LaunchVelocity)
{
	if (bFinaleStarted || CompletedFlips < Stages.Num()) return;
	bFinaleStarted = true;
	if (NarrationDirector) NarrationDirector->QueueSection(TEXT("Main.End"));
}

void AMainRingSequence::HandleSectionFinished(FName Section)
{
	if (Section == FName(TEXT("Main.Landing"))) bLandingNarrationStarted = true;
	if (Section != FName(TEXT("Main.End"))) return;
	bEndingFinished = true;
	ApplyGateState();
}

void AMainRingSequence::RestoreSavedState()
{
	ApplyGateState();
	SetActorTickEnabled(true);
}

void AMainRingSequence::HandleCompletion(FName GoalTag)
{
	if (bCelebrationCompleted || IsEndingPlaying()) return;
	APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Controller || !Player) return;
	Player->PrepareForMenu();
	Player->GetCharacterMovement()->StopMovementImmediately();
	Player->GetCharacterMovement()->DisableMovement();
	Controller->SetIgnoreMoveInput(true);
	Controller->SetIgnoreLookInput(true);
	CameraStart = Controller->PlayerCameraManager->GetCameraLocation();
	CameraStartRotation = Controller->PlayerCameraManager->GetCameraRotation();
	EndingCamera = GetWorld()->SpawnActor<ACameraActor>(CameraStart, CameraStartRotation);
	EndingCamera->Tags.Add(TEXT("RingOverview"));
	EndingCamera->GetCameraComponent()->SetFieldOfView(70.0f);
	Controller->SetViewTarget(EndingCamera);
	EndingUI = CreateWidget<ULoopEndingWidget>(Controller);
	EndingUI->AddToViewport(9000);
	EndingMedia = LoadObject<UMediaPlayer>(nullptr, TEXT("/Game/MegaJam2026/UI/Media/MP_LoopIntro.MP_LoopIntro"));
	if (EndingMedia)
	{
		EndingMedia->Close();
		EndingMedia->SetLooping(false);
		EndingMedia->OnEndReached.AddUniqueDynamic(this, &AMainRingSequence::HandleMovieFinished);
	}
	SetOverviewVisibility(true);
	EnterEndingPhase(1);
}

void AMainRingSequence::EnterEndingPhase(int32 Phase)
{
	EndingPhase = Phase;
	EndingElapsed = 0;
}

void AMainRingSequence::HandleMovieFinished()
{
	bMovieFinished = true;
}

void AMainRingSequence::TickEnding(float DeltaSeconds)
{
	EndingElapsed += DeltaSeconds;
	const float Ease = FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp(EndingElapsed / 8.0f, 0.0f, 1.0f));
	if (EndingPhase == 1)
	{
		const FVector Location = FMath::Lerp(CameraStart, RingCenter + FVector(0, -8000, OverviewHeight), Ease);
		EndingCamera->SetActorLocation(Location);
		const FRotator LookAt = (RingCenter - Location).Rotation();
		EndingCamera->SetActorRotation(FMath::Lerp(CameraStartRotation.Quaternion(), LookAt.Quaternion(), Ease));
		if (EndingElapsed >= 12.0f) EnterEndingPhase(2);
	}
	else if (EndingPhase == 2)
	{
		EndingUI->SetPresentation(FMath::Clamp(EndingElapsed, 0.0f, 1.0f), 0, 0);
		if (EndingElapsed >= 1.0f)
		{
			bMovieFinished = false;
			UMediaSource* Source = LoadObject<UMediaSource>(nullptr, TEXT("/Game/MegaJam2026/UI/Media/FMS_LoopIntro.FMS_LoopIntro"));
			if (!EndingMedia || !Source || !EndingMedia->OpenSource(Source)) bMovieFinished = true;
			EnterEndingPhase(3);
		}
	}
	else if (EndingPhase == 3)
	{
		const bool bReady = EndingMedia && (EndingMedia->IsPlaying() || EndingMedia->IsPaused());
		EndingUI->SetPresentation(bReady ? 1.0f - FMath::Clamp(EndingElapsed, 0.0f, 1.0f) : 1, 1, 0);
		if ((bMovieFinished && EndingElapsed > 1.0f) || EndingElapsed > 45.0f) EnterEndingPhase(4);
	}
	else if (EndingPhase == 4)
	{
		EndingUI->SetPresentation(FMath::Clamp(EndingElapsed, 0.0f, 1.0f), 1, 0);
		if (EndingElapsed >= 1.0f)
		{
			if (EndingMedia) EndingMedia->Close();
			EnterEndingPhase(5);
		}
	}
	else if (EndingPhase == 5)
	{
		const float Opacity = FMath::Min(FMath::Clamp(EndingElapsed / 0.8f, 0.0f, 1.0f), FMath::Clamp((4.5f - EndingElapsed) / 0.8f, 0.0f, 1.0f));
		EndingUI->SetPresentation(1, 0, Opacity);
		if (EndingElapsed >= 4.5f) EnterEndingPhase(6);
	}
	else if (EndingPhase == 6)
	{
		const float Progress = FMath::SmoothStep(0.0f, 1.0f, FMath::Clamp((EndingElapsed - 1.0f) / 4.0f, 0.0f, 1.0f));
		GetWorld()->GetSubsystem<ULoopWorldColorSubsystem>()->SetRestorationProgress(Progress);
		EndingUI->SetPresentation(1.0f - FMath::Clamp(EndingElapsed, 0.0f, 1.0f), 0, 0);
		if (EndingElapsed >= 8.0f) EnterEndingPhase(7);
	}
	else if (EndingPhase == 7)
	{
		EndingUI->SetPresentation(FMath::Clamp(EndingElapsed, 0.0f, 1.0f), 0, 0);
		if (EndingElapsed >= 1.0f) ReleasePlayer();
	}
	else if (EndingPhase == 8)
	{
		EndingUI->SetPresentation(1.0f - FMath::Clamp(EndingElapsed / 1.5f, 0.0f, 1.0f), 0, 0);
		if (EndingElapsed >= 1.5f)
		{
			EndingUI->RemoveFromParent();
			EndingUI = nullptr;
			EndingCamera->Destroy();
			EndingCamera = nullptr;
			bCelebrationCompleted = true;
			ApplyGateState();
			EnterEndingPhase(0);
		}
	}
}

void AMainRingSequence::ReleasePlayer()
{
	APlayerController* Controller = UGameplayStatics::GetPlayerController(this, 0);
	APlayerCharacter* Player = Cast<APlayerCharacter>(Controller->GetPawn());
	SetOverviewVisibility(false);
	const UStaticMeshComponent* Ground = FinaleBlock->FindComponentByClass<UStaticMeshComponent>();
	const FBox Bounds = Ground->GetStaticMesh()->GetBoundingBox();
	const FVector Surface = Ground->GetComponentTransform().TransformPosition(FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Max.Z));
	Player->PrepareForMenu();
	Player->GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
	Player->SetActorRotation(FRotator::ZeroRotator);
	Player->SetActorLocation(Surface + FVector(0, 0, 1800), false, nullptr, ETeleportType::TeleportPhysics);
	Player->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	CheckpointBlock = FinaleBlock->GetFName();
	Controller->SetViewTarget(Player);
	Controller->SetControlRotation(FRotator(-20, 90, 0));
	Controller->ResetIgnoreMoveInput();
	Controller->ResetIgnoreLookInput();
	EnterEndingPhase(8);
}

void AMainRingSequence::SetOverviewVisibility(bool bOverview)
{
	if (bOverview)
	{
		for (TActorIterator<AActor> Actor(GetWorld()); Actor; ++Actor)
		{
			TInlineComponentArray<UMeshComponent*> Meshes(*Actor);
			for (UMeshComponent* Mesh : Meshes)
			{
				if (!Mesh->IsVisible() || Actor->GetActorLocation().Z < 0) continue;
				UMaterialInterface* Material = Mesh->GetMaterial(0);
				if (Actor->ActorHasTag(TEXT("EndingUpperWater")) || (Material && Material->GetName() == TEXT("M_FlipWorldBed")))
				{
					HiddenWater.Add(Mesh);
					Mesh->SetVisibility(false);
				}
			}
			if (UExponentialHeightFogComponent* Fog = Actor->FindComponentByClass<UExponentialHeightFogComponent>())
			{
				FogDensities.Emplace(Fog, Fog->FogDensity);
				Fog->SetFogDensity(0.00002f);
			}
		}
	}
	else
	{
		for (auto& Mesh : HiddenWater) if (Mesh.IsValid()) Mesh->SetVisibility(true);
		const bool bRestored = GetWorld()->GetSubsystem<ULoopWorldColorSubsystem>()->IsWorldColorRestored();
		for (auto& Fog : FogDensities) if (Fog.Key.IsValid()) Fog.Key->SetFogDensity(Fog.Value * (bRestored ? 0.12f : 1.0f));
		HiddenWater.Empty();
		FogDensities.Empty();
	}
}

void AMainRingSequence::EndPlay(const EEndPlayReason::Type Reason)
{
	if (EndingMedia)
	{
		EndingMedia->OnEndReached.RemoveDynamic(this, &AMainRingSequence::HandleMovieFinished);
		EndingMedia->Close();
	}
	if (EndingUI) EndingUI->RemoveFromParent();
	SetOverviewVisibility(false);
	Super::EndPlay(Reason);
}
