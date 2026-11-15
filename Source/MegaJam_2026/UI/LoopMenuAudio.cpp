#include "LoopMenuAudio.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ULoopMenuAudio::ULoopMenuAudio()
{
	static ConstructorHelpers::FObjectFinder<USoundBase> Single(TEXT("/Game/Resources/Assets/Resources/UI/ui-button-aluminum-click-single-01.ui-button-aluminum-click-single-01"));
	static ConstructorHelpers::FObjectFinder<USoundBase> Double(TEXT("/Game/Resources/Assets/Resources/UI/ui-button-aluminum-click-double-01.ui-button-aluminum-click-double-01"));
	Click = Single.Object;
	Confirm = Double.Object;
}

void ULoopMenuAudio::ApplyToButton(UButton* Button, bool bConfirm) const
{
	if (!Button) return;
	FButtonStyle Style = Button->GetStyle();
	Style.PressedSlateSound.SetResourceObject(bConfirm ? Confirm.Get() : Click.Get());
	Button->SetStyle(Style);
}

void ULoopMenuAudio::Apply(UWidgetTree* Tree) const
{
	if (!Tree) return;
	Tree->ForEachWidget([this](UWidget* Widget)
	{
		ApplyToButton(Cast<UButton>(Widget));
	});
}
