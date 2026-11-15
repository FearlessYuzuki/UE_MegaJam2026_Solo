#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootstepAudioComponent.generated.h"

class ACharacter;
class UAudioComponent;
class UMaterialInterface;
class USoundBase;
class USoundSubmix;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EFootstepSurface : uint8
{
	Concrete,
	Grass,
	Water
};

UENUM(BlueprintType)
enum class EFootstepAudioEvent : uint8
{
	Step,
	Jump,
	Landing
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class MEGAJAM_2026_API UFootstepAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFootstepAudioComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category = "Footsteps")
	EFootstepSurface GetCurrentSurface() const;

	void PlayJumpSound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Sounds")
	TArray<TObjectPtr<USoundBase>> WaterSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Sounds")
	TArray<TObjectPtr<USoundBase>> GrassSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Sounds")
	TArray<TObjectPtr<USoundBase>> ConcreteSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Volume = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Water", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float WaterVolume = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Water")
	TObjectPtr<USoundSubmix> WaterReverbSubmix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Water", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WaterReverbSend = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Water", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float WaterGroundLayerVolume = 0.18f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps|Water", meta = (ClampMin = "200.0", ClampMax = "20000.0", Units = "Hz"))
	float WaterLowPassFrequency = 6000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps", meta = (ClampMin = "30.0", Units = "cm"))
	float StepDistance = 165.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps", meta = (ClampMin = "0.05", Units = "s"))
	float MinimumStepInterval = 0.18f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Footsteps")
	EFootstepSurface LastSurface = EFootstepSurface::Concrete;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Footsteps")
	int32 PlayedStepCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Footsteps")
	int32 PlayedJumpCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Footsteps")
	int32 PlayedLandingCount = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Footsteps")
	EFootstepAudioEvent LastAudioEvent = EFootstepAudioEvent::Step;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Footsteps")
	TObjectPtr<UAudioComponent> LastFootstepAudio;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Footsteps")
	TObjectPtr<UAudioComponent> LastWaterGroundAudio;

protected:
	virtual void BeginPlay() override;

private:
	static bool IsWaterMaterial(const UMaterialInterface* Material);
	static EFootstepSurface ClassifyMaterial(const UMaterialInterface* Material);
	bool IsOnWater(const FVector& FootLocation, const FVector& UpDirection) const;
	EFootstepSurface ResolveSurface(const FHitResult& Hit) const;
	void PlayFootstep();
	bool PlaySurfaceSound(EFootstepSurface Surface, EFootstepAudioEvent Event, float Gain, float Pitch);
	USoundBase* PickSound(const TArray<TObjectPtr<USoundBase>>& Sounds) const;

	UFUNCTION()
	void HandleLanded(const FHitResult& Hit);

	TWeakObjectPtr<ACharacter> Character;
	TArray<TWeakObjectPtr<UStaticMeshComponent>> WaterSurfaces;
	TWeakObjectPtr<USoundBase> PreviousSound;
	FVector PreviousLocation = FVector::ZeroVector;
	float DistanceSinceStep = 0.0f;
	float TimeSinceStep = 0.0f;
	bool bWasWalking = false;
	EFootstepSurface LastGroundSurface = EFootstepSurface::Concrete;
};
