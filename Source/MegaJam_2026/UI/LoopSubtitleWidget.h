#pragma once

#include "CoreMinimal.h"
#include "MegaJam_2026/UI/LoopInstructionWidget.h"
#include "LoopSubtitleWidget.generated.h"

class UFont;

UCLASS()
class MEGAJAM_2026_API ULoopSubtitleWidget : public ULoopInstructionWidget
{
	GENERATED_BODY()

public:
	ULoopSubtitleWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;

private:
	UPROPERTY()
	TObjectPtr<UFont> SubtitleFont;
};
