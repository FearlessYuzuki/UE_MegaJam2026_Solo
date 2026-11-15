#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MegaJam_2026/Settings/LoopSettings.h"
#include "LoopInstructionWidget.generated.h"

UCLASS()
class MEGAJAM_2026_API ULoopInstructionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gameplay Language", meta=(MultiLine=true))
	FText EnglishInstruction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Gameplay Language", meta=(MultiLine=true))
	FText ChineseInstruction;

	UFUNCTION(BlueprintCallable, Category="Gameplay Language")
	void SetInstruction(const FText& English, const FText& Chinese);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativePreConstruct() override;

private:
	UFUNCTION()
	void RefreshInstruction(ELoopGameplayLanguage Language);
};
