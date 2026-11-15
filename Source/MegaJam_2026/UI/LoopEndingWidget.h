#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoopEndingWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class MEGAJAM_2026_API ULoopEndingWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetPresentation(float BlackOpacity, float MovieOpacity, float TextOpacity);
protected:
	virtual void NativeOnInitialized() override;
private:
	UPROPERTY(Transient) TObjectPtr<UImage> Black;
	UPROPERTY(Transient) TObjectPtr<UImage> Movie;
	UPROPERTY(Transient) TObjectPtr<UTextBlock> Thanks;
};
