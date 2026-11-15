#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UInputMappingContext;
class ULoopMainMenuWidget;
class ULoopPauseMenuWidget;

UCLASS(abstract, config = "Game")
class MEGAJAM_2026_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Menus")
	TSubclassOf<ULoopMainMenuWidget> MainMenuClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Menus")
	TSubclassOf<ULoopPauseMenuWidget> PauseMenuClass;
	UFUNCTION(BlueprintCallable, Category="Menus") void TogglePauseMenu();

protected:
	UPROPERTY(EditAnywhere, category="Input|Input Mappings")
	TArray<TObjectPtr<UInputMappingContext>> InputMappingContexts;

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
};
