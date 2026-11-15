#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "GroundBlock.generated.h"

class UInstancedStaticMeshComponent;
class USoundBase;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
class UBlockCenterMarkerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroundBlockActivated, AActor*, ActivatingActor);

UCLASS()
class MEGAJAM_2026_API AGroundBlock : public AActor
{
	GENERATED_BODY()

public:
	AGroundBlock();
	void RestoreSavedState();
	void SetMenuColorVisible(bool bVisible);

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Lift Block")
	void StartLift();

	FName GetLiftRegionTag() const { return LiftRegionTag; }

	UFUNCTION(BlueprintPure, Category = "Lift Block")
	bool IsLifting() const { return bIsLifting; }

	UFUNCTION(BlueprintPure, Category = "Lift Block")
	bool IsActivated() const { return bIsActivated; }

	UPROPERTY(BlueprintAssignable, Category = "Lift Block")
	FOnGroundBlockActivated OnBlockActivated;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Lift Block")
	void ActivateMaterials(AActor* ActivatingActor);

	void RefreshAttachedSurface();
	void RebuildGrassInstances();
	void ApplySurfaceMaterials();
	FVector ResolveLiftDirection() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block")
	FName LiftRegionTag = TEXT("LiftRegion_A");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block")
	bool bRequiresLift = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block")
	bool bStartAtLiftTarget = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block", meta = (ClampMin = "0.0"))
	float LiftHeight = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block", meta = (ClampMin = "0.01"))
	float LiftDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block")
	bool bUsePlayerGravityForLift = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block")
	TObjectPtr<USoundBase> LiftSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Material", meta = (DisplayName = "Initial Material"))
	TObjectPtr<UMaterialInterface> WhiteMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Material")
	TObjectPtr<UMaterialInterface> LandMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Material")
	TObjectPtr<UMaterialInterface> GrassMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Material")
	bool bStartActivated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Grass")
	TObjectPtr<UStaticMesh> GrassMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Grass", meta = (ClampMin = "10.0", Units = "cm"))
	float GrassSpacing = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Grass", meta = (ClampMin = "0.05", ClampMax = "5.0"))
	float GrassScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lift Block|Grass")
	int32 GrassSeed = 17;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GroundMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TopSurface;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> GrassInstances;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBlockCenterMarkerComponent> CenterMarker;

	UPROPERTY(SaveGame) FVector LiftStartLocation;
	UPROPERTY(SaveGame) FVector LiftTargetLocation;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Block", meta = (AllowPrivateAccess = "true"))
	float LiftProgress = 0.0f;

	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Block", meta = (AllowPrivateAccess = "true"))
	bool bIsLifting = false;

	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Block", meta = (AllowPrivateAccess = "true"))
	bool bHasFinishedLift = false;

	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category = "Lift Block", meta = (AllowPrivateAccess = "true"))
	bool bIsActivated = false;

	double LiftStartTime = 0.0;
	bool bMenuColorVisible = false;
};
