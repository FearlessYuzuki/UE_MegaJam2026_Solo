#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoopSubtitleSubsystem.generated.h"

class ULoopSubtitleWidget;

UCLASS()
class MEGAJAM_2026_API ULoopSubtitleSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Subtitles", meta = (AdvancedDisplay = "Duration"))
	void ShowSubtitle(const FText& English, const FText& Chinese, float Duration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Tutorial|Subtitles")
	void ClearSubtitle();

	virtual void Deinitialize() override;

private:
	void HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	UPROPERTY(Transient)
	TObjectPtr<ULoopSubtitleWidget> Subtitle;

	TWeakObjectPtr<UWorld> TimerWorld;
	FTimerHandle SubtitleTimer;
};
