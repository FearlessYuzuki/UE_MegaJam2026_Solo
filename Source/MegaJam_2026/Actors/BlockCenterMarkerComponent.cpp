#include "Actors/BlockCenterMarkerComponent.h"

#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

UBlockCenterMarkerComponent::UBlockCenterMarkerComponent()
{
	SetMobility(EComponentMobility::Movable);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetCanEverAffectNavigation(false);
	SetCastShadow(false);
	SetReceivesDecals(false);
	SetRenderCustomDepth(false);
	SetCustomDepthStencilValue(0);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(
		TEXT("/Game/MegaJam2026/Materials/M_BlockCenterMarker.M_BlockCenterMarker"));
	if (Plane.Succeeded()) SetStaticMesh(Plane.Object);
	if (Material.Succeeded()) SetMaterial(0, Material.Object);
}

void UBlockCenterMarkerComponent::RefreshForSurface(UStaticMeshComponent* Surface)
{
	if (!Surface || !Surface->GetStaticMesh()) return;
	const FBox SurfaceBounds = Surface->GetStaticMesh()->GetBoundingBox();
	const FVector Scale = Surface->GetComponentScale().GetAbs().ComponentMax(FVector(0.001f));
	const float Diameter = FMath::Min(36.0f, FMath::Min(SurfaceBounds.GetSize().X * Scale.X, SurfaceBounds.GetSize().Y * Scale.Y) * 0.3f);
	SetRelativeLocation(FVector(SurfaceBounds.GetCenter().X, SurfaceBounds.GetCenter().Y, SurfaceBounds.Max.Z + 0.6f / Scale.Z));
	SetRelativeScale3D(FVector(Diameter / (100.0f * Scale.X), Diameter / (100.0f * Scale.Y), 1.0f));
}
