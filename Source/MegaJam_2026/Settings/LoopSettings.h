#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LoopSettings.generated.h"

UENUM(BlueprintType)
enum class ELoopGameplayLanguage : uint8
{
	English,
	Chinese
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoopLanguageChanged, ELoopGameplayLanguage, Language);

UCLASS(Config=GameUserSettings, BlueprintType)
class MEGAJAM_2026_API ULoopSettings : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category="Settings")
	static ULoopSettings* GetLoopSettings();

	UPROPERTY(Config, BlueprintReadOnly, Category="Settings")
	int32 MouseSensitivity = 100;

	UPROPERTY(Config, BlueprintReadOnly, Category="Settings")
	int32 GraphicsQuality = 2;

	UPROPERTY(Config, BlueprintReadOnly, Category="Settings")
	int32 FrameRateLimit = 120;

	UPROPERTY(Config, BlueprintReadOnly, Category="Settings")
	float MusicVolume = 1.0f;

	UFUNCTION(BlueprintCallable, Category="Settings")
	void SetMusicVolume(float Value, bool bSave = true);

	UPROPERTY(Config, BlueprintReadOnly, Category="Settings")
	ELoopGameplayLanguage GameplayLanguage = ELoopGameplayLanguage::English;

	UPROPERTY(BlueprintAssignable, Category="Settings")
	FLoopLanguageChanged OnGameplayLanguageChanged;

	UFUNCTION(BlueprintCallable, Category="Settings")
	void CommitSettings(int32 Sensitivity, int32 Quality, int32 FrameLimit, ELoopGameplayLanguage Language);

	UFUNCTION(BlueprintCallable, Category="Settings")
	void ApplyRenderingSettings();

	UFUNCTION(BlueprintPure, Category="Settings")
	float GetMouseSensitivityMultiplier() const;

	void Normalize();
};
