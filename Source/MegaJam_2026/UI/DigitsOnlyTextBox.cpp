#include "MegaJam_2026/UI/DigitsOnlyTextBox.h"
#include "Widgets/Input/SEditableTextBox.h"

TSharedRef<SWidget> UDigitsOnlyTextBox::RebuildWidget()
{
	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	MyEditableTextBlock->SetOnKeyCharHandler(FOnKeyChar::CreateUObject(this, &UDigitsOnlyTextBox::FilterCharacter));
	return Widget;
}

FReply UDigitsOnlyTextBox::FilterCharacter(const FGeometry& Geometry, const FCharacterEvent& Event)
{
	const TCHAR Character = Event.GetCharacter();
	return Character < TEXT(' ') || (Character >= TEXT('0') && Character <= TEXT('9'))
		? FReply::Unhandled() : FReply::Handled();
}
