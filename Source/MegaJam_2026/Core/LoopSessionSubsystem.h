#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LoopSessionSubsystem.generated.h"

class AMyPlayerController;
class ULoopMainMenuWidget;
class ULoopPauseMenuWidget;
class ULoopSaveGame;
class SBorder;
class UGameViewportClient;
class UFont;

UCLASS()
class MEGAJAM_2026_API ULoopSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	void InitializeController(AMyPlayerController* PlayerController);
	UFUNCTION(BlueprintCallable, Category="Game Session") void NewGame();
	UFUNCTION(BlueprintCallable, Category="Game Session") void StartMainChapter();
	UFUNCTION(BlueprintCallable, Category="Game Session") bool ContinueGame();
	UFUNCTION(BlueprintCallable, Category="Game Session") bool SaveCurrentGame();
	UFUNCTION(BlueprintPure, Category="Game Session") bool HasSavedGame() const;
	UFUNCTION(BlueprintCallable, Category="Game Session") void TogglePauseMenu();
	UFUNCTION(BlueprintCallable, Category="Game Session") void ResumeGame();
	UFUNCTION(BlueprintCallable, Category="Game Session") bool SaveAndExit();
	UFUNCTION(BlueprintCallable, Category="Game Session") void RestartLevel();
	UFUNCTION(BlueprintPure, Category="Game Session") bool IsAtMainMenu() const { return bAtMainMenu; }
	UFUNCTION(BlueprintPure, Category="Game Session") bool IsTutorialIntroPlaying() const { return IntroPhase != EIntroPhase::None; }
	UFUNCTION(BlueprintPure, Category="Game Session") float GetIntroFadeOpacity() const { return IntroFadeOpacity; }
	UFUNCTION(BlueprintPure, Category="Game Session") FText GetIntroTitle() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Session") FString SaveSlotName = TEXT("LoopProgress");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Game Session") bool bQuitOnSaveExit = false;

private:
	enum class EIntroPhase : uint8 { None, FadeOut, Blackout, Loading, Hold, Reveal, AwaitLanding };
	EIntroPhase IntroPhase = EIntroPhase::None;
	float IntroElapsed = 0.0f;
	double IntroPhaseStartedAt = 0.0;
	float IntroFadeOpacity = 0.0f;
	bool bIntroNarrationStarted = false;
	bool bMainChapterTransition = false;
	void BeginChapterTransition();
	UPROPERTY(Transient) TObjectPtr<UFont> IntroTitleFont;
	TSharedPtr<SBorder> IntroOverlay;
	TWeakObjectPtr<UGameViewportClient> IntroViewport;
	FTSTicker::FDelegateHandle IntroTicker;
	bool TickTutorialIntro(float DeltaSeconds);
	void BeginTutorialReveal();
	void PlayTutorialNarration();
	void ClearTutorialTransition();
	UPROPERTY(Transient) TObjectPtr<ULoopSaveGame> PendingSave;
	UPROPERTY(Transient) TObjectPtr<ULoopMainMenuWidget> MainMenu;
	UPROPERTY(Transient) TObjectPtr<ULoopPauseMenuWidget> PauseMenu;
	TWeakObjectPtr<AMyPlayerController> Controller;
	bool bAtMainMenu = false;
	bool bMenuOnNextLoad = true;
	bool bTransitioning = false;
	void ApplyPendingSave();
	void PrepareTravel();
	void ShowOpeningMenu();
	void SetOpeningSceneColor(bool bVisible);
};
