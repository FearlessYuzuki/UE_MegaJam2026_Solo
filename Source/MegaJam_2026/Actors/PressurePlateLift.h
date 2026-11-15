#pragma once

#include "CoreMinimal.h"
#include "Actors/PressurePlateBase.h"
#include "PressurePlateLift.generated.h"

class ALiftRegionController;

/** 地块升起请求已发出时广播。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLiftPlateTriggered, FName, RegionTag);

/**
 * 【红色 · 圆形】地块升起压力板。
 *
 * 踩下后向所有 LiftRegionTag 匹配的 ALiftRegionController 发 RequestLift()，
 * 由它们广播给挂同一 Tag 的 AGroundBlock 逐个升起。
 * 没有找到控制器时，退化为直接遍历场景里的 AGroundBlock 调 StartLift()。
 *
 * 一次性：地块升完就锁死。
 */
UCLASS(Blueprintable)
class MEGAJAM_2026_API APressurePlateLift : public APressurePlateBase
{
	GENERATED_BODY()

public:
	APressurePlateLift();

	UPROPERTY(BlueprintAssignable, Category = "Pressure Plate|Lift")
	FOnLiftPlateTriggered OnLiftTriggered;

	/** 手动重新收集一次场景里的地块控制器（运行时新增地块后用）。 */
	UFUNCTION(BlueprintCallable, Category = "Pressure Plate|Lift")
	void CacheLiftControllers();

protected:
	virtual void BeginPlay() override;
	virtual void ApplyPlateEffect(AActor* PressingActor) override;

	/** 要升起的地块分组名，需与 AGroundBlock / ALiftRegionController 上的 Tag 一致。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Lift")
	FName LiftRegionTag = TEXT("LiftRegion_A");

	/** 找不到控制器时是否直接驱动同 Tag 的 AGroundBlock。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Lift")
	bool bFallbackToGroundBlocks = true;

private:
	TArray<TWeakObjectPtr<ALiftRegionController>> CachedControllers;
};
