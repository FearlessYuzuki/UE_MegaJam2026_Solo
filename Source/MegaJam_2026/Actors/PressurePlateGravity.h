#pragma once

#include "CoreMinimal.h"
#include "Actors/PressurePlateBase.h"
#include "PressurePlateGravity.generated.h"

/** 重力翻转行为。 */
UENUM(BlueprintType)
enum class EGravityPlateMode : uint8
{
	/** 取反：正立 ↔ 上方世界，玩家可以来回穿。 */
	Toggle UMETA(DisplayName = "取反（上下世界互相切换）"),

	/** 强制把重力指向世界 +Z，玩家一定被"吸"到上方世界。 */
	ForceUp UMETA(DisplayName = "强制翻到上方世界"),

	/** 强制恢复正立（重力指向 -Z）。 */
	ForceDown UMETA(DisplayName = "强制恢复正立")
};

/** 重力翻转完成时广播，NewGravityDirection 为翻转后玩家受到的"重力指向"。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGravityPlateFlipped, AActor*, PlayerActor,
                                             FVector, NewGravityDirection);

/**
 * 【红色 · 方形】重力压力板。
 *
 * 踩下后改变玩家的重力方向，把玩家送到上方世界。
 * 复用 BasicBox 的整套重力设施：USourceCharacterMovementComponent::SetGravityDirection()
 * + APlayerCharacter::SetCameraAngle()（含 180° 镜头翻滚与输入锁）。
 *
 * 一次性：翻完就锁死，玩家在上下世界之间来回跑不会被反复翻转。
 */
UCLASS(Blueprintable)
class MEGAJAM_2026_API APressurePlateGravity : public APressurePlateBase
{
	GENERATED_BODY()

public:
	APressurePlateGravity();

	UPROPERTY(BlueprintAssignable, Category = "Pressure Plate|Gravity")
	FOnGravityPlateFlipped OnGravityFlipped;

protected:
	virtual void ApplyPlateEffect(AActor* PressingActor) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Gravity")
	EGravityPlateMode FlipMode = EGravityPlateMode::Toggle;

	/** 翻转后是否把玩家的移动速度清零，避免保留旧方向的速度导致穿模。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Gravity")
	bool bStopMovementOnFlip = true;
};
