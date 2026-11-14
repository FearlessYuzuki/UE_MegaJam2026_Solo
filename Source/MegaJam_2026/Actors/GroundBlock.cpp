#include "GroundBlock.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
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
}

void AGroundBlock::BeginPlay()
{
	Super::BeginPlay();

	GroundMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AGroundBlock::StartLift()
{
	if (!GetWorld() || !GetWorld()->IsGameWorld() || bIsLifting || bHasFinishedLift)
	{
		return;
	}

	LiftStartLocation = GetActorLocation();
	LiftTargetLocation = LiftStartLocation + GetActorUpVector() * FMath::Max(LiftHeight, 0.0f);
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
		SetActorTickEnabled(false);
		UE_LOG(LogTemp, Display, TEXT("Lift block %s finished: time=%.3f collision enabled"),
			*GetName(), GetWorld()->GetTimeSeconds());
	}
}
