#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RainAmbience.generated.h"

class AEmitter;
class UAudioComponent;
class USoundBase;

UCLASS()
class MEGAJAM_2026_API ARainAmbience : public AActor
{
	GENERATED_BODY()

public:
	ARainAmbience();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rain")
	TObjectPtr<UAudioComponent> RainAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain")
	TObjectPtr<USoundBase> RainSound;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Rain")
	TArray<TObjectPtr<AEmitter>> RainEmitters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Volume = 0.4f;

private:
	float CurrentVolume = 0.0f;
};
