#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundBlock.generated.h"

class USoundBase;
class UStaticMeshComponent;

UCLASS()
class MEGAJAM_2026_API AGroundBlock : public AActor
{
	GENERATED_BODY()

public:
	AGroundBlock();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Lift Block")
	void StartLift();

	FName GetLiftRegionTag() const { return LiftRegionTag; }

	UFUNCTION(BlueprintPure, Category = "Lift Block")
	bool IsLifting() const { return bIsLifting; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block")
	FName LiftRegionTag = TEXT("LiftRegion_A");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block", meta = (ClampMin = "0.0"))
	float LiftHeight = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block", meta = (ClampMin = "0.01"))
	float LiftDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block")
	TObjectPtr<USoundBase> LiftSound;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GroundMesh;

	FVector LiftStartLocation;
	FVector LiftTargetLocation;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Block", meta = (AllowPrivateAccess = "true"))
	float LiftProgress = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Block", meta = (AllowPrivateAccess = "true"))
	bool bIsLifting = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Block", meta = (AllowPrivateAccess = "true"))
	bool bHasFinishedLift = false;

	double LiftStartTime = 0.0;
};
