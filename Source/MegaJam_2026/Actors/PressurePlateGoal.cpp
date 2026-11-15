#include "Actors/PressurePlateGoal.h"
#include "Rendering/LoopWorldColorSubsystem.h"
#include "Engine/World.h"

#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

APressurePlateGoal::APressurePlateGoal()
{
	PlateColorIdle = FLinearColor(1.0f, 0.8f, 0.005f);
	PlateColorPressed = PlateColorIdle * 0.65f;
	PlateColorPressed.A = 1.0f;

	PlateSize = 100.0f;
	PressDepth = 5.0f;
	PressDuration = 0.15f;
	bOneShot = true;

	VictoryMessage = FText::FromString(TEXT("关卡目标达成！"));
}

void APressurePlateGoal::ApplyPlateEffect(AActor* PressingActor)
{
	if (bRestoreWorldColorOnReach)
	{
		if (ULoopWorldColorSubsystem* WorldColor = GetWorld()->GetSubsystem<ULoopWorldColorSubsystem>())
		{
			WorldColor->RestoreWorldColor();
		}
	}
	// 先广播，让蓝图拿到时机做结算（开 UI / 切关卡 / 停表）
	OnGoalReached.Broadcast(GoalTag);

	if (bShowVictoryMessage && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, FMath::Max(VictoryMessageDuration, 0.5f), FColor::Yellow,
		                                 FString::Printf(TEXT("[%s] %s"), *GoalTag.ToString(),
		                                                 *VictoryMessage.ToString()));
	}

	if (bLockPlayerInputOnReach)
	{
		if (const APawn* Pawn = Cast<APawn>(PressingActor))
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController()))
			{
				PlayerController->SetIgnoreMoveInput(true);
				PlayerController->SetIgnoreLookInput(true);
			}
		}
	}

	UE_LOG(LogTemp, Display, TEXT("[%s] 目标压力板达成：GoalTag=%s"), *GetName(), *GoalTag.ToString());
}
