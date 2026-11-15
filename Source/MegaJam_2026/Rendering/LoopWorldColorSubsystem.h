#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LoopWorldColorSubsystem.generated.h"

class UMaterialParameterCollection;
class UMaterialInstanceDynamic;
class UExponentialHeightFogComponent;
class ULightComponent;

UCLASS()
class MEGAJAM_2026_API ULoopWorldColorSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	ULoopWorldColorSubsystem();
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	UFUNCTION(BlueprintCallable, Category = "Loop|World Color")
	void RestoreWorldColor();
	UFUNCTION(BlueprintCallable, Category = "Loop|World Color")
	void SetRestorationProgress(float Progress);

	UFUNCTION(BlueprintCallable, Category = "Loop|World Color")
	void RestoreSavedState(bool bRestored);

	UFUNCTION(BlueprintPure, Category = "Loop|World Color")
	bool IsWorldColorRestored() const { return bWorldColorRestored; }

protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

private:
	void ApplyColor(float Saturation);

	UPROPERTY()
	TObjectPtr<UMaterialParameterCollection> ColorCollection;

	bool bWorldColorRestored = false;
	float ColorProgress = 0.0f;
	UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> WaterMaterials;
	TArray<TPair<TWeakObjectPtr<UExponentialHeightFogComponent>, float>> FogDensities;
	TArray<TPair<TWeakObjectPtr<ULightComponent>, float>> LightIntensities;
	float PulseElapsed = 2.5f;
	static constexpr float PulseDuration = 2.5f;
};
