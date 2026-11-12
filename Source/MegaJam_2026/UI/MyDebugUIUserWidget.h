#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyDebugUIUserWidget.generated.h"

class APlayerCharacter;
class UButton;
class UCameraComponent;

UCLASS()
class MEGAJAM_2026_API UMyDebugUIUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> FirstPersonButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ThirdPersonButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> KillGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ResumeGame;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnFirstPersonClicked();

	UFUNCTION()
	void OnThirdPersonClicked();

	UFUNCTION()
	void OnKillGameClicked();

	UFUNCTION()
	void OnResumeCliceked();
};
