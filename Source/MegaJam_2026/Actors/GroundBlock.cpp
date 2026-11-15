#include "GroundBlock.h"
#include "Actors/BlockCenterMarkerComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AGroundBlock::AGroundBlock()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	GroundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundMesh"));
	RootComponent = GroundMesh;
	GroundMesh->SetMobility(EComponentMobility::Movable);
	GroundMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	GroundMesh->SetGenerateOverlapEvents(false);
	GroundMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 0.4f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		GroundMesh->SetStaticMesh(CubeMesh.Object);
	}
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultWhiteMaterial(
		TEXT("/Game/MegaJam2026/Materials/LiftBlocks/M_BlockPureWhite.M_BlockPureWhite"));
	if (DefaultWhiteMaterial.Succeeded())
	{
		WhiteMaterial = DefaultWhiteMaterial.Object;
		GroundMesh->SetMaterial(0, WhiteMaterial);
	}

	TopSurface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopSurface"));
	TopSurface->SetupAttachment(GroundMesh);
	TopSurface->SetMobility(EComponentMobility::Movable);
	TopSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TopSurface->SetGenerateOverlapEvents(false);
	if (WhiteMaterial)
	{
		TopSurface->SetMaterial(0, WhiteMaterial);
	}
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		TopSurface->SetStaticMesh(PlaneMesh.Object);
	}

	CenterMarker = CreateDefaultSubobject<UBlockCenterMarkerComponent>(TEXT("CenterMarker"));
	CenterMarker->SetupAttachment(GroundMesh);

	GrassInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GrassInstances"));
	GrassInstances->SetupAttachment(GroundMesh);
	GrassInstances->SetUsingAbsoluteScale(true);
	GrassInstances->SetMobility(EComponentMobility::Movable);
	GrassInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GrassInstances->SetGenerateOverlapEvents(false);
	GrassInstances->SetCanEverAffectNavigation(false);
	GrassInstances->SetCastShadow(false);
	GrassInstances->SetCullDistances(8000, 12000);
	GrassInstances->SetVisibility(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultGrassMesh(
		TEXT("/Game/PN_FoliageCollection/Meshes/grassMesh/lowGrass_01_01_SM.lowGrass_01_01_SM"));
	if (DefaultGrassMesh.Succeeded())
	{
		GrassMesh = DefaultGrassMesh.Object;
	}

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(GroundMesh);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AGroundBlock::OnTriggerBeginOverlap);
	RefreshAttachedSurface();
}

void AGroundBlock::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	bIsActivated = bStartActivated;
	RefreshAttachedSurface();
	ApplySurfaceMaterials();
}

void AGroundBlock::ApplySurfaceMaterials()
{
	const bool bKeepColor = (bMenuColorVisible && ActorHasTag(TEXT("OpeningMenuColor")))
		|| ActorHasTag(TEXT("InvertedWorldColor"));
	UMaterialInterface* SideMaterial = bIsActivated ? LandMaterial.Get() : WhiteMaterial.Get();
	UMaterialInterface* SurfaceMaterial = bIsActivated ? GrassMaterial.Get() : WhiteMaterial.Get();
	if (SideMaterial)
	{
		GroundMesh->SetMaterial(0, SideMaterial);
	}
	if (SurfaceMaterial)
	{
		TopSurface->SetMaterial(0, SurfaceMaterial);
	}
	GrassInstances->SetVisibility(bIsActivated);
	if (GroundMesh)
	{
		GroundMesh->SetRenderCustomDepth(bKeepColor);
		GroundMesh->SetCustomDepthStencilValue(bKeepColor ? 1 : 0);
	}
	if (TopSurface)
	{
		TopSurface->SetRenderCustomDepth(bKeepColor);
		TopSurface->SetCustomDepthStencilValue(bKeepColor ? 1 : 0);
	}
	if (GrassInstances)
	{
		GrassInstances->SetRenderCustomDepth(bKeepColor);
		GrassInstances->SetCustomDepthStencilValue(bKeepColor ? 1 : 0);
	}
}

void AGroundBlock::SetMenuColorVisible(bool bVisible)
{
	bMenuColorVisible = bVisible;
	ApplySurfaceMaterials();
}

void AGroundBlock::RefreshAttachedSurface()
{
	if (!GroundMesh->GetStaticMesh())
	{
		return;
	}
	const FBox Bounds = GroundMesh->GetStaticMesh()->GetBoundingBox();
	const FVector Scale = GroundMesh->GetComponentScale().GetAbs();
	const FVector InverseScale(1.0f / FMath::Max(Scale.X, 0.001f),
		1.0f / FMath::Max(Scale.Y, 0.001f), 1.0f / FMath::Max(Scale.Z, 0.001f));
	TopSurface->SetRelativeLocation(FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Max.Z + InverseScale.Z * 0.2f));
	TopSurface->SetRelativeScale3D(FVector(Bounds.GetSize().X / 100.0f, Bounds.GetSize().Y / 100.0f, 1.0f));
	CenterMarker->RefreshForSurface(GroundMesh);
	TriggerBox->SetRelativeScale3D(InverseScale);
	TriggerBox->SetRelativeLocation(FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y, Bounds.Max.Z + InverseScale.Z * 45.0f));
	TriggerBox->SetBoxExtent(FVector(FMath::Max(Bounds.GetExtent().X * Scale.X - 12.0f, 1.0f),
		FMath::Max(Bounds.GetExtent().Y * Scale.Y - 12.0f, 1.0f), 40.0f));
	RebuildGrassInstances();
}

void AGroundBlock::RebuildGrassInstances()
{
	GrassInstances->ClearInstances();
	GrassInstances->SetStaticMesh(GrassMesh);
	GrassInstances->SetVisibility(bIsActivated);
	if (!GrassMesh || !GroundMesh->GetStaticMesh())
	{
		return;
	}
	const FBox Bounds = GroundMesh->GetStaticMesh()->GetBoundingBox();
	const FVector Scale = GroundMesh->GetComponentScale().GetAbs();
	GrassInstances->SetRelativeScale3D(FVector::OneVector);
	GrassInstances->SetRelativeLocation(FVector(Bounds.GetCenter().X, Bounds.GetCenter().Y,
		Bounds.Max.Z + 0.3f / FMath::Max(Scale.Z, 0.001f)));
	const FBox GrassBounds = GrassMesh->GetBoundingBox();
	const float InstanceScale = FMath::Clamp(GrassScale, 0.05f, 5.0f);
	const float Width = Bounds.GetSize().X * Scale.X;
	const float Depth = Bounds.GetSize().Y * Scale.Y;
	if (Width <= 0.0f || Depth <= 0.0f)
	{
		return;
	}
	const float Spacing = FMath::Max(GrassSpacing, 10.0f);
	const int32 Columns = FMath::Clamp(FMath::CeilToInt(Width / Spacing), 1, 32);
	const int32 Rows = FMath::Clamp(FMath::CeilToInt(Depth / Spacing), 1, 32);
	FRandomStream Random(GrassSeed);
	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Column = 0; Column < Columns; ++Column)
		{
			const float CellWidth = Width / Columns;
			const float CellDepth = Depth / Rows;
			const float PositionX = -Width * 0.5f + CellWidth * (Column + Random.FRandRange(0.45f, 0.55f));
			const float PositionY = -Depth * 0.5f + CellDepth * (Row + Random.FRandRange(0.45f, 0.55f));
			if (FMath::Square(PositionX) + FMath::Square(PositionY) < FMath::Square(30.0f))
			{
				continue;
			}
			const float Size = InstanceScale * Random.FRandRange(0.9f, 1.1f);
			const FTransform Instance(FRotator(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f),
				FVector(PositionX, PositionY, -GrassBounds.Min.Z * Size), FVector(Size));
			GrassInstances->AddInstance(Instance);
		}
	}
}

FVector AGroundBlock::ResolveLiftDirection() const
{
	if (bUsePlayerGravityForLift)
	{
		if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (const UCharacterMovementComponent* Movement = Player->FindComponentByClass<UCharacterMovementComponent>())
			{
				return -Movement->GetGravityDirection().GetSafeNormal();
			}
		}
	}
	return GetActorUpVector();
}

void AGroundBlock::BeginPlay()
{
	Super::BeginPlay();
	if (bRequiresLift && bStartAtLiftTarget && !bStartActivated)
	{
		SetActorLocation(GetActorLocation() - ResolveLiftDirection() * FMath::Max(LiftHeight, 0.0f));
	}

	bIsActivated = bStartActivated;
	bHasFinishedLift = bStartActivated || !bRequiresLift;
	RefreshAttachedSurface();
	ApplySurfaceMaterials();
	GroundMesh->SetCollisionEnabled(bHasFinishedLift ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	TriggerBox->SetCollisionEnabled(bHasFinishedLift && !bIsActivated ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AGroundBlock::StartLift()
{
	if (!bRequiresLift || !GetWorld() || !GetWorld()->IsGameWorld() || bIsLifting || bHasFinishedLift)
	{
		return;
	}

	LiftStartLocation = GetActorLocation();
	const FVector LiftDirection = ResolveLiftDirection();
	if (FVector::DotProduct(GetActorUpVector(), LiftDirection) < 0.999f)
	{
		SetActorRotation(FQuat::FindBetweenNormals(GetActorUpVector(), LiftDirection) * GetActorQuat());
	}
	RefreshAttachedSurface();
	LiftTargetLocation = LiftStartLocation + LiftDirection * FMath::Max(LiftHeight, 0.0f);
	LiftStartTime = GetWorld()->GetTimeSeconds();
	LiftProgress = 0.0f;
	bIsLifting = true;
	GroundMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(true);

	if (LiftSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LiftSound, LiftStartLocation);
	}

	UE_LOG(LogTemp, Display, TEXT("Lift block %s started: region=%s time=%.3f targetZ=%.1f"),
		*GetName(), *LiftRegionTag.ToString(), LiftStartTime, LiftTargetLocation.Z);
}

void AGroundBlock::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsLifting)
	{
		return;
	}

	const double Elapsed = GetWorld()->GetTimeSeconds() - LiftStartTime;
	LiftProgress = FMath::Clamp(static_cast<float>(Elapsed / FMath::Max(LiftDuration, 0.01f)), 0.0f, 1.0f);
	const float EasedProgress = FMath::SmoothStep(0.0f, 1.0f, LiftProgress);
	SetActorLocation(FMath::Lerp(LiftStartLocation, LiftTargetLocation, EasedProgress));

	if (LiftProgress >= 1.0f)
	{
		bIsLifting = false;
		bHasFinishedLift = true;
		GroundMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		TArray<AActor*> OverlappingActors;
		TriggerBox->GetOverlappingActors(OverlappingActors);
		for (AActor* OverlappingActor : OverlappingActors)
		{
			ActivateMaterials(OverlappingActor);
		}
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Display, TEXT("Lift block %s finished: time=%.3f collision enabled"),
			*GetName(), GetWorld()->GetTimeSeconds());
	}
}

void AGroundBlock::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ActivateMaterials(OtherActor);
}

void AGroundBlock::RestoreSavedState()
{
	LiftStartTime = GetWorld()->GetTimeSeconds() - LiftProgress * FMath::Max(LiftDuration, 0.01f);
	RefreshAttachedSurface();
	ApplySurfaceMaterials();
	GroundMesh->SetCollisionEnabled(bHasFinishedLift ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	TriggerBox->SetCollisionEnabled(bHasFinishedLift && !bIsActivated ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	SetActorTickEnabled(bIsLifting);
}

void AGroundBlock::ActivateMaterials(AActor* ActivatingActor)
{
	const APawn* Player = Cast<APawn>(ActivatingActor);
	if (!bHasFinishedLift || bIsActivated || !Player || !Player->IsPlayerControlled() || !LandMaterial || !GrassMaterial)
	{
		return;
	}
	const UCharacterMovementComponent* Movement = Player->FindComponentByClass<UCharacterMovementComponent>();
	if (!Movement || FVector::DotProduct(-Movement->GetGravityDirection(), GetActorUpVector()) < 0.9f)
	{
		return;
	}
	bIsActivated = true;
	ApplySurfaceMaterials();
	GrassInstances->SetVisibility(true);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnBlockActivated.Broadcast(ActivatingActor);
}
