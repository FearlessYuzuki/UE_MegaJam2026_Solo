#pragma once

#include "CoreMinimal.h"
#include "Components/EditableTextBox.h"
#include "DigitsOnlyTextBox.generated.h"

UCLASS()
class MEGAJAM_2026_API UDigitsOnlyTextBox : public UEditableTextBox
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FReply FilterCharacter(const FGeometry& Geometry, const FCharacterEvent& Event);
};
