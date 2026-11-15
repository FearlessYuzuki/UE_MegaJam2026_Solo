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
	void RestoreSavedState();

	UFUNCTION(BlueprintCallable, Category = "Lift Region")
	void RequestLift();

	UPROPERTY(BlueprintAssignable, Category = "Lift Region")
	FOnRegionLiftRequested OnLiftRequested;

	/** 供压力板等外部对象按 Tag 匹配（与 AGroundBlock::GetLiftRegionTag 保持一致）。 */
	UFUNCTION(BlueprintPure, Category = "Lift Region")
	FName GetLiftRegionTag() const { return LiftRegionTag; }

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
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Region", meta = (AllowPrivateAccess = "true"))
	bool bHasTriggered = false;

	FTimerHandle AutoStartTimer;
};
