#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "WorldsBasicActor.generated.h"

UCLASS()
class MEGAJAM_2026_API AWorldsBasicActor : public AActor
{
	GENERATED_BODY()

public:
	AWorldsBasicActor();

protected:
	UFUNCTION()
	void OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                              const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BaseBox;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;
};
