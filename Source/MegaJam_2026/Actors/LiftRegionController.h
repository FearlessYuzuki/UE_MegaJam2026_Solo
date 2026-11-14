#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiftRegionController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRegionLiftRequested);

UCLASS()
class MEGAJAM_2026_API ALiftRegionController : public AActor
{
	GENERATED_BODY()

public:
	ALiftRegionController();

	UFUNCTION(BlueprintCallable, Category = "Lift Region")
	void RequestLift();

	UPROPERTY(BlueprintAssignable, Category = "Lift Region")
	FOnRegionLiftRequested OnLiftRequested;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Region")
	FName LiftRegionTag = TEXT("LiftRegion_A");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Region")
	bool bAutoStart = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Region", meta = (ClampMin = "0.01"))
	float AutoStartDelay = 3.0f;

private:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Region", meta = (AllowPrivateAccess = "true"))
	bool bHasTriggered = false;

	FTimerHandle AutoStartTimer;
};
