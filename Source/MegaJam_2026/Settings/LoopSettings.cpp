#include "MegaJam_2026/Settings/LoopSettings.h"
#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "Audio/MusicPlaylistActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"

ULoopSettings* ULoopSettings::GetLoopSettings()
{
	ULoopSettings* Settings = GetMutableDefault<ULoopSettings>();
	Settings->Normalize();
	return Settings;
}

void ULoopSettings::Normalize()
{
	MouseSensitivity = FMath::Clamp(MouseSensitivity, 1, 200);
	MusicVolume = FMath::Clamp(MusicVolume, 0.0f, 1.0f);
	GraphicsQuality = FMath::Clamp(GraphicsQuality, 0, 2);
	const TArray<int32> SupportedLimits = {0, 30, 60, 90, 120, 144, 165, 240};
	if (!SupportedLimits.Contains(FrameRateLimit))
	{
		FrameRateLimit = 120;
	}
	if (GameplayLanguage != ELoopGameplayLanguage::English && GameplayLanguage != ELoopGameplayLanguage::Chinese)
	{
		GameplayLanguage = ELoopGameplayLanguage::English;
	}
}

void ULoopSettings::CommitSettings(int32 Sensitivity, int32 Quality, int32 FrameLimit, ELoopGameplayLanguage Language)
{
	const ELoopGameplayLanguage PreviousLanguage = GameplayLanguage;
	MouseSensitivity = Sensitivity;
	GraphicsQuality = Quality;
	FrameRateLimit = FrameLimit;
	GameplayLanguage = Language;
	Normalize();
	ApplyRenderingSettings();
	SaveConfig();
	if (PreviousLanguage != GameplayLanguage)
	{
		OnGameplayLanguageChanged.Broadcast(GameplayLanguage);
	}
}

void ULoopSettings::ApplyRenderingSettings()
{
	Normalize();
	if (GEngine)
	{
		if (UGameUserSettings* UserSettings = GEngine->GetGameUserSettings())
		{
			UserSettings->SetOverallScalabilityLevel(GraphicsQuality);
			UserSettings->SetFrameRateLimit(static_cast<float>(FrameRateLimit));
			UserSettings->ApplyNonResolutionSettings();
			UserSettings->SaveSettings();
		}
	}
	for (const TCHAR* VariableName : {TEXT("r.Lumen.DiffuseIndirect.Allow"), TEXT("r.Lumen.Reflections.Allow")})
	{
		if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(VariableName))
		{
			Variable->Set(GraphicsQuality == 2 ? 1 : 0, ECVF_SetByGameSetting);
		}
	}
}

float ULoopSettings::GetMouseSensitivityMultiplier() const
{
	return FMath::Clamp(MouseSensitivity, 1, 200) / 100.0f;
}

void ULoopSettings::SetMusicVolume(float Value, bool bSave)
{
	MusicVolume = FMath::Clamp(Value, 0.0f, 1.0f);
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (UWorld* World = Context.World(); World && World->IsGameWorld())
			{
				for (TActorIterator<AMusicPlaylistActor> Music(World); Music; ++Music) Music->RefreshVolume();
			}
		}
	}
	if (bSave) SaveConfig();
}
