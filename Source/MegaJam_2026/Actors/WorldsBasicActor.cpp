#include "WorldsBasicActor.h"

#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AWorldsBasicActor::AWorldsBasicActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BaseBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseBox"));
	RootComponent = BaseBox;
	BaseBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BaseBox->SetRelativeScale3D(FVector(2.0f, 1.0f, 0.5f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BaseBox->SetStaticMesh(CubeMesh.Object);
	}

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f, 150.0f));
	TriggerBox->SetBoxExtent(FVector(100.0f, 50.0f, 50.0f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AWorldsBasicActor::OnTriggerBoxBeginOverlap);
}

void AWorldsBasicActor::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                 bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && Pawn->IsPlayerControlled() && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::White,
			TEXT("Player is overlapping with Actor Upper"));
	}
}
