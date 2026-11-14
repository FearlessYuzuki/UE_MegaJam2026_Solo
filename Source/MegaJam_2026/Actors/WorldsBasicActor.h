#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "WorldsBasicActor.generated.h"

class APawn;
class UMaterialInterface;

/** 玩家进入/离开触发盒时广播，bEntered = true 表示进入。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldsBasicPlayerOverlap, AActor*, PlayerActor,
                                             bool, bEntered);

/**
 * 可踩的“正方柱体”底座 + 上方触发盒。
 * 玩家与触发盒重叠时，改变本 Actor 的 BaseBox 材质。
 */
UCLASS()
class MEGAJAM_2026_API AWorldsBasicActor : public AActor
{
	GENERATED_BODY()

public:
	AWorldsBasicActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** 仅在相应自动尺寸开关开启时刷新；默认保留蓝图手动编辑的形状。 */
	UFUNCTION(BlueprintCallable, Category = "WorldsBasic")
	void RefreshBaseShape();

	/** 把脚下方块切换为高亮/普通材质，供蓝图或调试使用。 */
	UFUNCTION(BlueprintCallable, Category = "WorldsBasic")
	void SetFootBlockHighlighted(bool bHighlighted);

	/** 玩家进入/离开触发盒。 */
	UPROPERTY(BlueprintAssignable, Category = "WorldsBasic")
	FOnWorldsBasicPlayerOverlap OnPlayerOverlapChanged;

protected:
	virtual void BeginPlay() override;

	virtual bool ShouldFlipPlayerOnOverlap() const;

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                              const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 蓝图事件：玩家踩进来 / 离开。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "WorldsBasic")
	void BP_OnPlayerEntered(AActor* PlayerActor, UStaticMeshComponent* FootBlock);

	UFUNCTION(BlueprintImplementableEvent, Category = "WorldsBasic")
	void BP_OnPlayerExited(AActor* PlayerActor, UStaticMeshComponent* FootBlock);

	// ---------------------------------------------------------------------
	// 形状设置：底座是一个底边等宽、高度更大的正方柱体
	// ---------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Shape",
		meta = (ToolTip = "开启后依据 BaseWidth / BaseHeight 自动调整底座；关闭时保留组件手动缩放。"))
	bool bAutoSizeBase = false;

	/** 底座水平边长（厘米，正方形截面）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Shape",
		meta = (ClampMin = "10.0", EditCondition = "bAutoSizeBase"))
	float BaseWidth = 200.0f;

	/** 底座高度（厘米）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Shape",
		meta = (ClampMin = "10.0", EditCondition = "bAutoSizeBase"))
	float BaseHeight = 400.0f;

	/** 触发盒半尺寸（底座局部空间，仅 bAutoFitTriggerToBase 开启时生效）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Shape",
		meta = (EditCondition = "bAutoFitTriggerToBase"))
	FVector TriggerExtent = FVector(140.0f, 140.0f, 100.0f);

	/** 触发盒中心相对底座顶面的额外 Z 偏移（底座局部空间，仅 bAutoFitTriggerToBase 开启时生效）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Shape",
		meta = (EditCondition = "bAutoFitTriggerToBase"))
	float TriggerHeightOffset = 0.0f;

	/**
	 * 是否让触发盒自动贴合底座顶面。
	 * 关闭后代码不再碰触发盒的 Location / Extent，可以完全在蓝图里手动摆放。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Shape")
	bool bAutoFitTriggerToBase = false;

	// ---------------------------------------------------------------------
	// 材质设置
	// ---------------------------------------------------------------------

	/** 平时使用的材质（留空则保持模型自带材质）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Material")
	TObjectPtr<UMaterialInterface> IdleMaterial;

	/** 被踩到时使用的材质。留空时会退化为对当前材质做颜色染色（需 bUseOverlapTint）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Material")
	TObjectPtr<UMaterialInterface> OverlapMaterial;

	/** 目标材质槽索引。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Material", meta = (ClampMin = "0"))
	int32 MaterialSlot = 0;

	/** 是否用 OverlapTint 给 OverlapMaterial 染色（材质需有 TintParameterName 参数）。
	 *  默认开启：即使 OverlapMaterial 留空，也会直接给当前材质（例如引擎自带 BasicShapeMaterial 的 Color 参数）染色。*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Material")
	bool bUseOverlapTint = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Material")
	FLinearColor OverlapTint = FLinearColor(1.0f, 0.25f, 0.1f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Material")
	FName TintParameterName = TEXT("Color");

	// ---------------------------------------------------------------------
	// 行为设置
	// ---------------------------------------------------------------------

	/** 玩家离开后是否恢复原材质。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Behaviour")
	bool bRestoreOnEndOverlap = true;

	/** 是否打印屏幕调试信息。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldsBasic|Behaviour")
	bool bShowDebugMessages = true;

private:
	/** 应用/恢复某个方块组件的材质。 */
	void ApplyMaterialToBlock(UStaticMeshComponent* Target, bool bHighlighted);
	void RestoreBlock(UStaticMeshComponent* Target);
	void RestoreAllBlocks();

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseBox;

	UPROPERTY(EditAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** 当前处于触发盒内的玩家，以及各个玩家脚下的方块。 */
	TSet<TWeakObjectPtr<APawn>> OverlappingPawns;
	TMap<TWeakObjectPtr<APawn>, TWeakObjectPtr<UStaticMeshComponent>> PawnFootBlocks;

	/** 被改过材质的组件 -> 原始材质（按槽位）。 */
	TMap<TObjectPtr<UStaticMeshComponent>, TArray<TObjectPtr<UMaterialInterface>>> OriginalMaterials;
};
