#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "MyRespawnActor.generated.h"

UCLASS()
class MEGAJAM_2026_API AMyRespawnActor : public AActor
{
	GENERATED_BODY()

public:
	AMyRespawnActor();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> BaseBox;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                              const FHitResult& SweepResult);

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
