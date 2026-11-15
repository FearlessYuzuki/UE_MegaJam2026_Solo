#include "MegaJam_2026/Settings/LoopGameplayLanguageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void ULoopGameplayLanguageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ULoopSettings* Settings = ULoopSettings::GetLoopSettings();
	Settings->ApplyRenderingSettings();
	Settings->OnGameplayLanguageChanged.AddUniqueDynamic(this, &ULoopGameplayLanguageSubsystem::HandleLanguageChanged);
}

void ULoopGameplayLanguageSubsystem::Deinitialize()
{
	ULoopSettings::GetLoopSettings()->OnGameplayLanguageChanged.RemoveDynamic(this, &ULoopGameplayLanguageSubsystem::HandleLanguageChanged);
	Super::Deinitialize();
}

FText ULoopGameplayLanguageSubsystem::GetInstructionText(const FText& English, const FText& Chinese) const
{
	return ULoopSettings::GetLoopSettings()->GameplayLanguage == ELoopGameplayLanguage::Chinese && !Chinese.IsEmpty()
		? Chinese : English;
}

USoundBase* ULoopGameplayLanguageSubsystem::GetNarrationSound(USoundBase* English, USoundBase* Chinese) const
{
	return ULoopSettings::GetLoopSettings()->GameplayLanguage == ELoopGameplayLanguage::Chinese && Chinese
		? Chinese : English;
}

UAudioComponent* ULoopGameplayLanguageSubsystem::PlayNarration(USoundBase* English, USoundBase* Chinese)
{
	USoundBase* Sound = GetNarrationSound(English, Chinese);
	return Sound ? UGameplayStatics::SpawnSound2D(this, Sound) : nullptr;
}

void ULoopGameplayLanguageSubsystem::HandleLanguageChanged(ELoopGameplayLanguage Language)
{
	OnGameplayLanguageChanged.Broadcast(Language);
}
