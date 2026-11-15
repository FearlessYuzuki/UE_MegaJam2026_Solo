#pragma once

#include "CoreMinimal.h"
#include "Actors/PressurePlateBase.h"
#include "PressurePlateGoal.generated.h"

/** 目标压力板达成时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoalPlateReached, FName, GoalTag);

/**
 * 【黄色 · 方形】关卡完成压力板。
 *
 * 踩下即视为达成目标：广播 OnGoalReached / OnPlatePressed，
 * 可选在屏幕上打出通关提示、锁存后不再响应。
 *
 * 真正的"结算"逻辑（打开结算 UI、切关卡、停表）留在蓝图里接 OnGoalReached，
 * 这样改玩法不用重编 C++。
 */
UCLASS(Blueprintable)
class MEGAJAM_2026_API APressurePlateGoal : public APressurePlateBase
{
	GENERATED_BODY()

public:
	APressurePlateGoal();

	UPROPERTY(BlueprintAssignable, Category = "Pressure Plate|Goal")
	FOnGoalPlateReached OnGoalReached;

protected:
	virtual void ApplyPlateEffect(AActor* PressingActor) override;

	/** 多目标关卡时用来区分是哪一块目标板达成了。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Goal")
	FName GoalTag = TEXT("MainGoal");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Goal")
	bool bRestoreWorldColorOnReach = true;

	/** 达成时是否在屏幕上打一行字（GameJam 阶段的廉价反馈）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Goal")
	bool bShowVictoryMessage = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Goal")
	FText VictoryMessage;

	/** 提示文字停留时长（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Goal",
		meta = (ClampMin = "0.5", Units = "s", EditCondition = "bShowVictoryMessage"))
	float VictoryMessageDuration = 5.0f;

	/** 是否同时把玩家输入锁掉，防止通关后还能继续跑。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Goal")
	bool bLockPlayerInputOnReach = false;
};
