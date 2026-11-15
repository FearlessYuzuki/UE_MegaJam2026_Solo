#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoopMenuAudio.generated.h"

class UWidgetTree;
class UButton;
class USoundBase;

UCLASS()
class MEGAJAM_2026_API ULoopMenuAudio : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	ULoopMenuAudio();
	void Apply(UWidgetTree* Tree) const;
	void ApplyToButton(UButton* Button, bool bConfirm = false) const;

private:
	UPROPERTY() TObjectPtr<USoundBase> Click;
	UPROPERTY() TObjectPtr<USoundBase> Confirm;
};
