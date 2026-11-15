#include "Actors/TutorialOpeningSequence.h"

#include "Actors/GroundBlock.h"
#include "Actors/PressurePlateLift.h"
#include "Actors/PressurePlateLaunch.h"
#include "Actors/PressurePlateGravity.h"
#include "Actors/PressurePlateGoal.h"
#include "UI/LoopNarrationDirector.h"
#include "Core/LoopSessionSubsystem.h"
#include "Characters/PlayerCharacter.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ATutorialOpeningSequence::ATutorialOpeningSequence()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
}

void ATutorialOpeningSequence::BeginPlay()
{
	Super::BeginPlay();
	if (OpeningPlate)
	{
		PlateTargetLocation = OpeningPlate->GetActorLocation();
		OpeningPlate->OnPlatePressed.AddUniqueDynamic(this, &ATutorialOpeningSequence::HandlePlatePressed);
		ApplyPlateState();
	}
	if (!IntroBlocks.IsEmpty())
	{
		if (IntroBlocks[0]) IntroBlocks[0]->OnBlockActivated.AddUniqueDynamic(this, &ATutorialOpeningSequence::HandleFirstSurface);
		if (IntroBlocks.Last()) IntroBlocks.Last()->OnBlockActivated.AddUniqueDynamic(this, &ATutorialOpeningSequence::HandleLastIntroSurface);
	}
	if (LaunchPlate) LaunchPlate->OnLaunchFired.AddUniqueDynamic(this, &ATutorialOpeningSequence::HandleLaunch);
	if (GravityPlate) GravityPlate->OnGravityFlipped.AddUniqueDynamic(this, &ATutorialOpeningSequence::HandleFlip);
	if (CompletionPlate) CompletionPlate->OnGoalReached.AddUniqueDynamic(this, &ATutorialOpeningSequence::HandleGoal);
	if (NarrationDirector) NarrationDirector->OnSectionFinished.AddUniqueDynamic(this, &ATutorialOpeningSequence::HandleSectionFinished);
	ApplyGoalState();
}

void ATutorialOpeningSequence::ApplyPlateState()
{
	if (!OpeningPlate) return;
	const float Progress = bPlateReady ? 1.0f : FMath::Clamp(PlateRiseElapsed / FMath::Max(PlateRiseDuration, 0.01f), 0.0f, 1.0f);
	const FVector Start = PlateTargetLocation - OpeningPlate->GetActorUpVector() * PlateRiseHeight;
	OpeningPlate->SetActorLocation(FMath::Lerp(Start, PlateTargetLocation, FMath::SmoothStep(0.0f, 1.0f, Progress)));
	OpeningPlate->SetActorEnableCollision(bPlateReady);
}

void ATutorialOpeningSequence::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const ULoopSessionSubsystem* Session = GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>();
	if (!OpeningPlate || (Session && Session->IsAtMainMenu())) return;
	if (!bLanded)
	{
		const ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (!Player || !Player->GetCharacterMovement()->IsMovingOnGround()) return;
		bLanded = true;
		TimeSinceLanding = 0.0f;
		return;
	}
	if (!bPlateReady)
	{
		TimeSinceLanding += DeltaSeconds;
		if (TimeSinceLanding < LandingDelay) return;
		if (!bIntroBlocksRequested)
		{
			bIntroBlocksRequested = true;
			for (AGroundBlock* Block : IntroBlocks)
			{
				if (Block) Block->StartLift();
			}
		}
		PlateRiseElapsed = FMath::Min(TimeSinceLanding - LandingDelay, PlateRiseDuration);
		bPlateReady = PlateRiseElapsed >= PlateRiseDuration;
		ApplyPlateState();
		if (bPlateReady && !IntroBlocks.IsEmpty()) QueueNarration(TEXT("Tutorial.GroundRaised"));
	}
	UpdateInvertedRecovery();
	if (bFlipHintPlayed && !bExitHintPlayed)
	{
		const ACharacter* Player = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
		if (Player && Player->GetCharacterMovement()->GetGravityDirection().Z > 0.9f
			&& Player->GetActorLocation().Z > 0.0f && Player->GetCharacterMovement()->IsMovingOnGround())
		{
			bExitHintPlayed = true;
			QueueNarration(TEXT("Tutorial.Exit"));
		}
	}
}

void ATutorialOpeningSequence::HandlePlatePressed(AActor* PressingActor)
{
	if (!bPlateReady || bRemainingRaised) return;
	bRemainingRaised = true;
	QueueNarration(TEXT("Tutorial.Lift"));
	for (AGroundBlock* Block : RemainingBlocks)
	{
		if (Block) Block->StartLift();
	}
}

void ATutorialOpeningSequence::RestoreSavedState()
{
	ApplyPlateState();
	ApplyGoalState();
	SetActorTickEnabled(true);
}

void ATutorialOpeningSequence::QueueNarration(FName Section)
{
	if (NarrationDirector) NarrationDirector->QueueSection(Section);
}

void ATutorialOpeningSequence::HandleFirstSurface(AActor* ActivatingActor)
{
	if (bGroundHintPlayed) return;
	bGroundHintPlayed = true;
	QueueNarration(TEXT("Tutorial.Trace"));
}

void ATutorialOpeningSequence::HandleLastIntroSurface(AActor* ActivatingActor)
{
	if (bPlateHintPlayed) return;
	bPlateHintPlayed = true;
	QueueNarration(TEXT("Tutorial.PlateIntro"));
}

void ATutorialOpeningSequence::HandleLaunch(AActor* LaunchedActor, FVector LaunchVelocity)
{
	if (bLaunchHintPlayed) return;
	bLaunchHintPlayed = true;
	QueueNarration(TEXT("Tutorial.Launch"));
}

void ATutorialOpeningSequence::HandleFlip(AActor* PlayerActor, FVector GravityDirection)
{
	if (bFlipHintPlayed) return;
	bFlipHintPlayed = true;
	if (!InvertedBlocks.IsEmpty() && InvertedBlocks[0]) InvertedCheckpoint = InvertedBlocks[0]->GetFName();
	QueueNarration(TEXT("Tutorial.Flip"));
}

void ATutorialOpeningSequence::UpdateInvertedRecovery()
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!bFlipHintPlayed || !Player || InvertedBlocks.IsEmpty()) return;
	UCharacterMovementComponent* Movement = Player->GetCharacterMovement();
	if (Movement->GetGravityDirection().Z < 0.9f) return;
	AGroundBlock* Checkpoint = nullptr;
	for (AGroundBlock* Block : InvertedBlocks)
	{
		if (!IsValid(Block)) continue;
		if (Block->GetFName() == InvertedCheckpoint) Checkpoint = Block;
		if (Movement->IsMovingOnGround() && Movement->CurrentFloor.HitResult.GetActor() == Block)
		{
			InvertedCheckpoint = Block->GetFName();
			return;
		}
	}
	if (!Checkpoint)
	{
		Checkpoint = InvertedBlocks[0];
		if (!IsValid(Checkpoint)) return;
		InvertedCheckpoint = Checkpoint->GetFName();
	}
	const UStaticMeshComponent* Ground = Checkpoint->FindComponentByClass<UStaticMeshComponent>();
	if (!Ground || !Ground->GetStaticMesh()) return;
	const FBox Bounds = Ground->GetStaticMesh()->GetBoundingBox();
	const FVector Surface = Ground->GetComponentTransform().TransformPosition(FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Max.Z));
	const FVector Gravity = Movement->GetGravityDirection();
	const FVector Feet = Player->GetActorLocation() + Gravity * Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	if (FVector::DotProduct(Feet - Surface, Gravity) <= RecoveryFallDistance) return;
	const ACharacter* DefaultPlayer = Player->GetClass()->GetDefaultObject<ACharacter>();
	const float HalfHeight = DefaultPlayer->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * Player->GetCapsuleComponent()->GetShapeScale();
	Player->PrepareForMenu();
	Movement->SetMovementMode(MOVE_Falling);
	Movement->StopMovementImmediately();
	Player->SetActorLocation(Surface - Gravity * (HalfHeight + 5.0f), false, nullptr, ETeleportType::TeleportPhysics);
	Player->UnCrouch();
	if (APlayerController* Controller = Cast<APlayerController>(Player->GetController()))
	{
		if (Controller->PlayerCameraManager)
		{
			Controller->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 0.3f, FLinearColor::Black);
		}
	}
}

void ATutorialOpeningSequence::ApplyGoalState()
{
	if (!CompletionPlate) return;
	CompletionPlate->SetActorHiddenInGame(!bGoalArmed);
	CompletionPlate->SetActorEnableCollision(bGoalArmed);
}

void ATutorialOpeningSequence::HandleSectionFinished(FName Section)
{
	if (Section != FName(TEXT("Tutorial.Exit"))) return;
	bGoalArmed = true;
	ApplyGoalState();
}

void ATutorialOpeningSequence::HandleGoal(FName GoalTag)
{
	if (!bGoalArmed) return;
	if (ULoopSessionSubsystem* Session = GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>())
	{
		Session->StartMainChapter();
	}
}
