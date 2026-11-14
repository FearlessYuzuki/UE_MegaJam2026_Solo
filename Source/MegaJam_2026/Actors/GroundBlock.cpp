#include "GroundBlock.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AGroundBlock::AGroundBlock()
{
	PrimaryActorTick.bCanEverTick = false;

	GroundMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GroundMesh"));
	RootComponent = GroundMesh;
	GroundMesh->SetMobility(EComponentMobility::Static);
	GroundMesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	GroundMesh->SetGenerateOverlapEvents(false);
	GroundMesh->SetRelativeScale3D(FVector(4.0f, 4.0f, 0.4f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		GroundMesh->SetStaticMesh(CubeMesh.Object);
	}
}
