#include "Actors/PressurePlateBase.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

APressurePlateBase::APressurePlateBase()
{
	// 逐帧开销：只在按下/回弹动画期间开启，静止时立刻关闭
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultPressSound(
		TEXT("/Game/MegaJam2026/Audio/PressurePlates/S_PressurePlate_Click2.S_PressurePlate_Click2"));
	if (DefaultPressSound.Succeeded())
	{
		PressSound = DefaultPressSound.Object;
	}

	PlateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlateMesh"));
	PlateMesh->SetupAttachment(RootComponent);
	PlateMesh->SetMobility(EComponentMobility::Movable);
	// 纯视觉件：不挡路、不产生重叠，踩上去的是 BasicBox 的 TriggerBox
	PlateMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlateMesh->SetGenerateOverlapEvents(false);
	PlateMesh->SetCastShadow(true);
	PlateMesh->SetRenderCustomDepth(true);
	PlateMesh->SetCustomDepthStencilValue(1);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		PlateMesh->SetStaticMesh(CubeMesh.Object);
	}

	// 默认板面材质保留 Color 染色参数，并提供轻微自发光
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> PlateMaterial(
		TEXT("/Game/MegaJam2026/Materials/M_PressurePlateGlow.M_PressurePlateGlow"));
	if (PlateMaterial.Succeeded())
	{
		PlateMesh->SetMaterial(0, PlateMaterial.Object);
	}

	// BasicBox 的重叠只用来告诉我们"玩家踩上来了"，不接管重力翻转
	bUseOverlapTint = false;
	bShowDebugMessages = false;

	// 触发盒贴合板子尺寸，避免 BasicBox 默认 140 半宽导致隔空触发
	bAutoFitTriggerToBase = true;
	TriggerExtent = FVector(PlateSize * 0.5f + TriggerPadding, PlateSize * 0.5f + TriggerPadding,
	                        TriggerHalfHeight);
}

void APressurePlateBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// 让编辑器里也能看到正确的尺寸与颜色，拖进关卡就能直接摆
	RefreshPlateTransform();
	ApplyPlateColor();
}

void APressurePlateBase::BeginPlay()
{
	Super::BeginPlay();

	// 复用 BasicBox 的玩家重叠广播，不重复写一套检测逻辑
	OnPlayerOverlapChanged.AddUniqueDynamic(this, &APressurePlateBase::HandlePlateOverlap);

	RefreshPlateTransform();
	ApplyPlateColor();
}

bool APressurePlateBase::ShouldFlipPlayerOnOverlap() const
{
	return false;
}

// ---------------------------------------------------------------------------
// 重叠
// ---------------------------------------------------------------------------

void APressurePlateBase::HandlePlateOverlap(AActor* PlayerActor, bool bEntered)
{
	if (bEntered)
	{
		++OverlapCount;
		if (!bIsPressed)
		{
			Press(PlayerActor);
		}
	}
	else
	{
		OverlapCount = FMath::Max(0, OverlapCount - 1);
		if (OverlapCount == 0 && bIsPressed)
		{
			Release(PlayerActor);
		}
	}
}

void APressurePlateBase::Press(AActor* PressingActor)
{
	if (bIsPressed || !CanPressAgain())
	{
		return;
	}

	bIsPressed = true;
	bHasFired = true;
	bAnimating = true;
	SetActorTickEnabled(true);

	ApplyPlateColor();
	PlayPlateSound(PressSound);
	ApplyPlateEffect(PressingActor);

	OnPlatePressed.Broadcast(PressingActor);
	BP_OnPlatePressed(PressingActor);
}

void APressurePlateBase::Release(AActor* ReleasingActor)
{
	if (!bIsPressed)
	{
		return;
	}

	// 一次性板子：保持按下外观，不再回弹（目标板/重力板踩完就该一直是"已使用"状态）
	if (bOneShot && bHasFired)
	{
		return;
	}

	bIsPressed = false;
	bAnimating = true;
	SetActorTickEnabled(true);

	ApplyPlateColor();
	PlayPlateSound(ReleaseSound);

	OnPlateReleased.Broadcast(ReleasingActor);
	BP_OnPlateReleased(ReleasingActor);
}

void APressurePlateBase::ResetPlate()
{
	bHasFired = false;
	bIsPressed = false;
	OverlapCount = 0;
	PressAlpha = 0.0f;
	bAnimating = false;
	SetActorTickEnabled(false);

	RefreshPlateTransform();
	ApplyPlateColor();
}

void APressurePlateBase::RestoreSavedState()
{
	bIsPressed = bOneShot && bHasFired;
	PressAlpha = bIsPressed ? 1.0f : 0.0f;
	OverlapCount = 0;
	bAnimating = false;
	SetActorTickEnabled(false);
	RefreshPlateTransform();
	ApplyPlateColor();
}

// ---------------------------------------------------------------------------
// 动画
// ---------------------------------------------------------------------------

void APressurePlateBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TickPressAnimation(DeltaSeconds);
}

void APressurePlateBase::TickPressAnimation(float DeltaSeconds)
{
	if (!bAnimating)
	{
		return;
	}

	const float Step = DeltaSeconds / FMath::Max(PressDuration, 0.01f);
	PressAlpha = FMath::Clamp(PressAlpha + (bIsPressed ? Step : -Step), 0.0f, 1.0f);
	RefreshPlateTransform();

	const bool bFinished = bIsPressed ? PressAlpha >= 1.0f : PressAlpha <= 0.0f;
	if (bFinished)
	{
		bAnimating = false;
		SetActorTickEnabled(false);
	}
}

void APressurePlateBase::RefreshPlateTransform()
{
	if (!PlateMesh)
	{
		return;
	}

	// 板子底面 = BaseBox 网格包围盒顶面 + 偏移。
	// 用网格本地包围盒而不是世界尺寸，这样 BaseBox 被缩放时板子会跟着缩放贴合。
	float SurfaceZ = PlateSurfaceOffset;
	if (bAutoAlignToBaseTop && RootComponent)
	{
		if (const UStaticMeshComponent* BaseComponent = Cast<UStaticMeshComponent>(RootComponent))
		{
			const UStaticMesh* BaseMesh = BaseComponent->GetStaticMesh();
			if (BaseMesh)
			{
				SurfaceZ = BaseMesh->GetBoundingBox().Max.Z + PlateSurfaceOffset;
			}
		}
	}

	// 引擎基础方块 100cm，缩放 = 实际尺寸 / 100
	const float SizeScale = FMath::Max(PlateSize, 10.0f) / 100.0f;
	const float ThicknessScale = FMath::Max(PlateThickness, 1.0f) / 100.0f;
	PlateMesh->SetRelativeScale3D(FVector(SizeScale, SizeScale, ThicknessScale));

	const float HalfThickness = FMath::Max(PlateThickness, 1.0f) * 0.5f;
	PlateRestLocation = FVector(0.0f, 0.0f, SurfaceZ + HalfThickness);
	PlateMesh->SetRelativeLocation(PlateRestLocation - FVector(0.0f, 0.0f, PressDepth * PressAlpha));

	// 触发盒跟随板子尺寸，改 PlateSize 后不用手动调
	TriggerExtent = FVector(PlateSize * 0.5f + TriggerPadding, PlateSize * 0.5f + TriggerPadding,
	                        TriggerHalfHeight);
	bAutoFitTriggerToBase = true;
	RefreshBaseShape();
}

void APressurePlateBase::ApplyPlateColor()
{
	if (!PlateMesh)
	{
		return;
	}

	const APressurePlateBase* DefaultPlate = GetClass()->GetDefaultObject<APressurePlateBase>();
	UMaterialInterface* SourceMaterial = DefaultPlate && DefaultPlate->PlateMesh
		? DefaultPlate->PlateMesh->GetMaterial(0)
		: PlateMesh->GetMaterial(0);

	// MID 只创建一次：比起每次重叠都 new 一个 MID，这样没有每帧的材质对象分配
	if (!PlateMID || PlateMID->Parent != SourceMaterial)
	{
		PlateMID = PlateMesh->CreateDynamicMaterialInstance(0, SourceMaterial,
		                                                    FName(TEXT("PressurePlateColorMID")));
	}

	if (!PlateMID)
	{
		return;
	}

	PlateMID->SetVectorParameterValue(PlateColorParameter, bIsPressed ? PlateColorPressed : PlateColorIdle);
	PlateMID->SetScalarParameterValue(TEXT("GlowStrength"), bIsPressed ? 1.8f : 3.0f);
	PlateMesh->SetRenderCustomDepth(true);
	PlateMesh->SetCustomDepthStencilValue(1);
}

void APressurePlateBase::PlayPlateSound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
	}
}
