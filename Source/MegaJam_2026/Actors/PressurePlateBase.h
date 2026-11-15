#pragma once

#include "CoreMinimal.h"
#include "Actors/WorldsBasicActor.h"
#include "PressurePlateBase.generated.h"

class UMaterialInstanceDynamic;
class UStaticMeshComponent;
class USoundBase;

/** 压力板被踩下时广播，PressingActor 为踩板子的 Pawn。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPressurePlatePressed, AActor*, PressingActor);

/** 压力板回弹时广播。一次性压力板不会触发此事件。 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPressurePlateReleased, AActor*, ReleasingActor);

/**
 * 压力板基类。
 *
 * 直接继承 BasicBox（AWorldsBasicActor），复用它的：
 *   - BaseBox / TriggerBox 组件
 *   - 玩家重叠检测与 OnPlayerOverlapChanged 广播
 *   - 材质与调试设施
 *
 * 本类只负责"按下/回弹"的状态机、视觉位移和事件派发；
 * 具体玩法效果由子类覆写 ApplyPlateEffect() 实现，四种颜色由子类构造函数写死默认值。
 *
 * 性能说明：Tick 默认关闭，只在按下/回弹的 PressDuration 内短暂开启，
 * 静止时完全不产生逐帧开销。
 */
UCLASS(Abstract, Blueprintable)
class MEGAJAM_2026_API APressurePlateBase : public AWorldsBasicActor
{
	GENERATED_BODY()

public:
	APressurePlateBase();
	void RestoreSavedState();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** 清除"已触发"锁存，让一次性压力板可以再次使用（例如玩家重生后）。 */
	UFUNCTION(BlueprintCallable, Category = "Pressure Plate")
	void ResetPlate();

	UFUNCTION(BlueprintPure, Category = "Pressure Plate")
	bool IsPressed() const { return bIsPressed; }

	/** 一次性压力板触发过之后返回 false。 */
	UFUNCTION(BlueprintPure, Category = "Pressure Plate")
	bool CanPressAgain() const { return !(bOneShot && bHasFired); }

	/** 当前站在板子上的 Pawn 数量。 */
	UFUNCTION(BlueprintPure, Category = "Pressure Plate")
	int32 GetOverlapCount() const { return OverlapCount; }

	UPROPERTY(BlueprintAssignable, Category = "Pressure Plate")
	FOnPressurePlatePressed OnPlatePressed;

	UPROPERTY(BlueprintAssignable, Category = "Pressure Plate")
	FOnPressurePlateReleased OnPlateReleased;

protected:
	virtual void BeginPlay() override;

	/**
	 * 基类版永久返回 false：BasicBox 原本"一踩上去就翻重力"的行为太激进，
	 * 压力板要在子类里、按自己的时机精准翻转一次。
	 */
	virtual bool ShouldFlipPlayerOnOverlap() const override;

	/** 子类实现真正的玩法效果。调用时机：外观已按下、声音已播放之后。 */
	virtual void ApplyPlateEffect(AActor* PressingActor) {}

	/** 绑定到 BasicBox 的 OnPlayerOverlapChanged。 */
	UFUNCTION()
	void HandlePlateOverlap(AActor* PlayerActor, bool bEntered);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pressure Plate", meta = (DisplayName = "On Plate Pressed"))
	void BP_OnPlatePressed(AActor* PressingActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pressure Plate", meta = (DisplayName = "On Plate Released"))
	void BP_OnPlateReleased(AActor* ReleasingActor);

	void Press(AActor* PressingActor);
	void Release(AActor* ReleasingActor);

	/** 依据 PlateSize / PlateThickness / PressAlpha 重算板子的相对变换与触发盒尺寸。 */
	void RefreshPlateTransform();

	/** 把当前颜色写进板子材质的 PlateColorParameter 参数（只创建一次 MID）。 */
	void ApplyPlateColor();

	void PlayPlateSound(USoundBase* Sound) const;

	// ---------------------------------------------------------------------
	// 组件
	// ---------------------------------------------------------------------

	/** 会上下位移的那块板子。本身不参与碰撞，纯粹是视觉件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlateMesh;

	// ---------------------------------------------------------------------
	// 尺寸
	// ---------------------------------------------------------------------

	/** 板子水平边长（厘米）。方形板为边长，圆形板为直径。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape",
		meta = (ClampMin = "10.0", Units = "cm"))
	float PlateSize = 90.0f;

	/** 板子厚度（厘米）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape",
		meta = (ClampMin = "1.0", Units = "cm"))
	float PlateThickness = 10.0f;

	/** 板子底面相对底座顶面的额外抬升（厘米），自动对齐时叠加。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape",
		meta = (ClampMin = "0.0", Units = "cm"))
	float PlateSurfaceOffset = 0.0f;

	/** 开启后板子底面自动贴在 BaseBox 网格包围盒的顶面；关闭则只用 PlateSurfaceOffset。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape")
	bool bAutoAlignToBaseTop = true;

	/** 按下时下沉的距离（厘米）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape",
		meta = (ClampMin = "0.0", Units = "cm"))
	float PressDepth = 4.0f;

	/** 按下 / 回弹的动画时长（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape",
		meta = (ClampMin = "0.01", Units = "s"))
	float PressDuration = 0.12f;

	/** 触发盒水平方向比板子每边多出的余量（厘米），避免玩家踩在边缘踩不到。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape",
		meta = (ClampMin = "0.0", Units = "cm"))
	float TriggerPadding = 15.0f;

	/** 触发盒半高（厘米）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Shape",
		meta = (ClampMin = "10.0", Units = "cm"))
	float TriggerHalfHeight = 80.0f;

	// ---------------------------------------------------------------------
	// 行为
	// ---------------------------------------------------------------------

	/**
	 * true = 只触发一次，之后保持按下外观、不再响应（目标板 / 重力板 / 升地块板）。
	 * false = 可反复踩（弹射板）。
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Behaviour")
	bool bOneShot = false;

	// ---------------------------------------------------------------------
	// 视觉
	// ---------------------------------------------------------------------

	/** 未按下时的颜色。默认低饱和度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Visual")
	FLinearColor PlateColorIdle = FLinearColor(0.35f, 0.35f, 0.35f, 1.0f);

	/** 按下时的颜色（通常是同色系压暗一档）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Visual")
	FLinearColor PlateColorPressed = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

	/** 材质里的 Vector 参数名，引擎自带 BasicShapeMaterial 用的是 Color。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Visual")
	FName PlateColorParameter = TEXT("Color");

	// ---------------------------------------------------------------------
	// 音频
	// ---------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Audio")
	TObjectPtr<USoundBase> PressSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pressure Plate|Audio")
	TObjectPtr<USoundBase> ReleaseSound;

private:
	void TickPressAnimation(float DeltaSeconds);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PlateMID;

	FVector PlateRestLocation = FVector::ZeroVector;
	UPROPERTY(SaveGame) float PressAlpha = 0.0f;
	UPROPERTY(SaveGame) bool bIsPressed = false;
	UPROPERTY(SaveGame) bool bHasFired = false;
	bool bAnimating = false;
	int32 OverlapCount = 0;
};
