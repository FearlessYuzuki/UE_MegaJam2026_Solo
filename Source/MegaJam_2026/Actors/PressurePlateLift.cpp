#include "Actors/PressurePlateLift.h"

#include "Actors/GroundBlock.h"
#include "Actors/LiftRegionController.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

APressurePlateLift::APressurePlateLift()
{
	PlateColorIdle = FLinearColor(1.0f, 0.008f, 0.025f);
	PlateColorPressed = PlateColorIdle * 0.65f;
	PlateColorPressed.A = 1.0f;

	// 圆形板：换成引擎自带圆柱
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PlateMesh->SetStaticMesh(CylinderMesh.Object);
	}

	PlateSize = 90.0f;
	PlateThickness = 12.0f;
	PressDepth = 6.0f;
	PressDuration = 0.18f;
	bOneShot = true;
}

void APressurePlateLift::BeginPlay()
{
	Super::BeginPlay();
	CacheLiftControllers();
}

void APressurePlateLift::CacheLiftControllers()
{
	CachedControllers.Reset();

	if (!GetWorld())
	{
		return;
	}

	for (TActorIterator<ALiftRegionController> It(GetWorld()); It; ++It)
	{
		if (It->GetLiftRegionTag() == LiftRegionTag)
		{
			CachedControllers.Add(*It);
		}
	}
}

void APressurePlateLift::ApplyPlateEffect(AActor* PressingActor)
{
	int32 RequestedCount = 0;

	for (const TWeakObjectPtr<ALiftRegionController>& WeakController : CachedControllers)
	{
		if (ALiftRegionController* Controller = WeakController.Get())
		{
			Controller->RequestLift();
			++RequestedCount;
		}
	}

	if (RequestedCount == 0 && bFallbackToGroundBlocks && GetWorld())
	{
		// 场景里没放控制器：直接驱动地块，保证板子不会"踩了没反应"
		for (TActorIterator<AGroundBlock> It(GetWorld()); It; ++It)
		{
			if (It->GetLiftRegionTag() == LiftRegionTag)
			{
				It->StartLift();
				++RequestedCount;
			}
		}

		UE_LOG(LogTemp, Warning,
		       TEXT("[%s] 按 Tag '%s' 没找到 ALiftRegionController，已回退为直接驱动 %d 个 AGroundBlock。"),
		       *GetName(), *LiftRegionTag.ToString(), RequestedCount);
	}

	OnLiftTriggered.Broadcast(LiftRegionTag);
}
