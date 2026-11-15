#pragma once

#include "CoreMinimal.h"
#include "Actors/PressurePlateBase.h"
#include "PressurePlateLaunch.generated.h"

/** 弹射发生时广播，LaunchVelocity 为施加后的完整速度。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLaunchPlateFired, AActor*, LaunchedActor,
                                             FVector, LaunchVelocity);

/**
 * 【绿色 · 方形】弹射压力板。
 *
 * 踩下后沿玩家当前的"上方向"（= -重力方向）弹射。
 * 关键点：不是用 LaunchCharacter 的 world Z，而是按 -GravityDirection 计算，
 * 所以重力被翻转的玩家站在上方世界踩它，也会朝上方世界的"上"弹。
 *
 * 默认可反复踩（bOneShot = false）：玩家被弹飞离开触发盒 → 板子自动回弹 →
 * 落回来再踩一次就再弹一次。每次"进入"只弹一次，站在板上不会持续施加推力。
 * 想变成"只能弹一次"就把 bOneShot 勾上，之后用 ResetPlate() 重新武装。
 */
UCLASS(Blueprintable)
class MEGAJAM_2026_API APressurePlateLaunch : public APressurePlateBase
{
	GENERATED_BODY()

public:
	APressurePlateLaunch();

	UPROPERTY(BlueprintAssignable, Category = "Pressure Plate|Launch")
	FOnLaunchPlateFired OnLaunchFired;

protected:
	virtual void ApplyPlateEffect(AActor* PressingActor) override;

	/** 沿"上方向"的弹射速度（厘米/秒）。玩家跳跃初速约 420，500 以上才有明显弹射感。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Launch",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float LaunchSpeed = 1200.0f;

	/** 保留多少原有水平速度（1 = 全保留，0 = 垂直起跳）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Launch",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LateralVelocityRetain = 1.0f;

	/** 弹射后强制切成 Falling，保证真的离地（走路状态下改 Velocity 会被摩擦吃掉）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Launch")
	bool bForceFalling = true;
};
