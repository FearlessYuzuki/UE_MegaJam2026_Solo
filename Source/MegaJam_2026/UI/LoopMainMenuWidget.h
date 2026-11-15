#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoopMainMenuWidget.generated.h"

UCLASS()
class MEGAJAM_2026_API ULoopMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Menu")
	FName OpeningCameraTag = TEXT("OpeningUICamera");

	UFUNCTION(BlueprintCallable, Category="Menu")
	static ULoopMainMenuWidget* ShowMainMenu(APlayerController* PlayerController, TSubclassOf<ULoopMainMenuWidget> MenuClass);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual bool IsPauseMenu() const { return false; }
	bool IsSettingsOpen() const;

private:
	UFUNCTION() void StartNewGame();
	UFUNCTION() void ContinueGame();
	UFUNCTION() void QuitGame();
	TWeakObjectPtr<AActor> PreviousViewTarget;
	TWeakObjectPtr<AActor> OpeningViewTarget;

	UFUNCTION()
	void OpenSettings();
	UFUNCTION()
	void RestoreMenu();
};
