#include "MegaJam_2026/Core/MyRespawnActor.h"

#include "MegaJam_2026/Characters/PlayerCharacter.h"
#include "Engine/Engine.h"

AMyRespawnActor::AMyRespawnActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BaseBox = CreateDefaultSubobject<UStaticMeshComponent>(FName("BaseBox"));
	RootComponent = BaseBox;

	BaseBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(FName("TriggerBox"));
	TriggerBox->SetupAttachment(BaseBox);
	TriggerBox->SetCollisionProfileName(FName("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMyRespawnActor::OnTriggerBoxBeginOverlap);
}

void AMyRespawnActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.RemoveDynamic(this, &AMyRespawnActor::OnTriggerBoxBeginOverlap);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMyRespawnActor::OnTriggerBoxBeginOverlap);
		UE_LOG(LogTemp, Warning, TEXT("[CP] BeginPlay rebound TriggerBox delegate: %s (bound=%d)"),
		       *TriggerBox->GetName(), TriggerBox->OnComponentBeginOverlap.IsBound() ? 1 : 0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[CP] TriggerBox is null at BeginPlay!"));
	}
}

void AMyRespawnActor::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                               AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                               int32 OtherBodyIndex,
                                               bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("[CP] OnTriggerBoxBeginOverlap: %s (Class=%s)"),
	       *GetNameSafe(OtherActor), OtherActor ? *OtherActor->GetClass()->GetName() : TEXT("null"));

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->SetSpawnPoint(this);
		UE_LOG(LogTemp, Warning, TEXT("[CP] Respawn point set to %s @ %s"), *GetName(),
		       *GetActorLocation().ToString());
	}
}

void AMyRespawnActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("[CP] NotifyActorBeginOverlap: %s (Class=%s)"),
	       *GetNameSafe(OtherActor), OtherActor ? *OtherActor->GetClass()->GetName() : TEXT("null"));

	FString Overlapped;
	for (UActorComponent* Comp : GetComponents())
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Comp))
		{
			if (Prim->IsOverlappingActor(OtherActor))
			{
				Overlapped += Prim->GetName() + TEXT(" ");
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[CP] Overlapping components: [%s]"), *Overlapped);

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		Player->SetSpawnPoint(this);
		UE_LOG(LogTemp, Warning, TEXT("[CP] (actor-level) Respawn point set to %s"), *GetName());
	}
}
