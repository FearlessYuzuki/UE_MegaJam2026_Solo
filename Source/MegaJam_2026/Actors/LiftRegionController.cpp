#include "LiftRegionController.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GroundBlock.h"
#include "TimerManager.h"

ALiftRegionController::ALiftRegionController()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
}

void ALiftRegionController::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<AGroundBlock> Block(GetWorld()); Block; ++Block)
	{
		if (Block->GetLiftRegionTag() == LiftRegionTag)
		{
			OnLiftRequested.AddUniqueDynamic(*Block, &AGroundBlock::StartLift);
		}
	}

	if (bAutoStart)
	{
		GetWorldTimerManager().SetTimer(AutoStartTimer, this, &ALiftRegionController::RequestLift,
			FMath::Max(AutoStartDelay, 0.01f), false);
	}
}

void ALiftRegionController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(AutoStartTimer);
	OnLiftRequested.Clear();
	Super::EndPlay(EndPlayReason);
}

void ALiftRegionController::RequestLift()
{
	if (!GetWorld() || !GetWorld()->IsGameWorld() || bHasTriggered)
	{
		return;
	}

	bHasTriggered = true;
	GetWorldTimerManager().ClearTimer(AutoStartTimer);
	UE_LOG(LogTemp, Display, TEXT("Lift region %s broadcast: time=%.3f"),
		*LiftRegionTag.ToString(), GetWorld()->GetTimeSeconds());
	OnLiftRequested.Broadcast();
}

void ALiftRegionController::RestoreSavedState()
{
	if (bHasTriggered) GetWorldTimerManager().ClearTimer(AutoStartTimer);
}
