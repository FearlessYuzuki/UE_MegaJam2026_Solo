#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MegaJam_2026/Settings/LoopSettings.h"
#include "LoopGameplayLanguageSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class MEGAJAM_2026_API ULoopGameplayLanguageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, Category="Gameplay Language")
	FLoopLanguageChanged OnGameplayLanguageChanged;

	UFUNCTION(BlueprintPure, Category="Gameplay Language")
	FText GetInstructionText(const FText& English, const FText& Chinese) const;

	UFUNCTION(BlueprintPure, Category="Gameplay Language")
	USoundBase* GetNarrationSound(USoundBase* English, USoundBase* Chinese) const;

	UFUNCTION(BlueprintCallable, Category="Gameplay Language")
	UAudioComponent* PlayNarration(USoundBase* English, USoundBase* Chinese);

private:
	UFUNCTION()
	void HandleLanguageChanged(ELoopGameplayLanguage Language);
};
