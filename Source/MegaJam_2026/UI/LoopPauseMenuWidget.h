#pragma once

#include "CoreMinimal.h"
#include "UI/LoopMainMenuWidget.h"
#include "LoopPauseMenuWidget.generated.h"

UCLASS()
class MEGAJAM_2026_API ULoopPauseMenuWidget : public ULoopMainMenuWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	virtual bool IsPauseMenu() const override { return true; }
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

private:
	TMap<FName, float> HoverPhases;
	UFUNCTION() void Resume();
	UFUNCTION() void SaveAndExit();
	UFUNCTION() void Restart();
};
