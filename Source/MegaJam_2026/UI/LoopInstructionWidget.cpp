#include "MegaJam_2026/UI/LoopInstructionWidget.h"
#include "Components/TextBlock.h"

void ULoopInstructionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ULoopSettings* Settings = ULoopSettings::GetLoopSettings();
	Settings->OnGameplayLanguageChanged.AddUniqueDynamic(this, &ULoopInstructionWidget::RefreshInstruction);
	RefreshInstruction(Settings->GameplayLanguage);
}

void ULoopInstructionWidget::NativeDestruct()
{
	ULoopSettings::GetLoopSettings()->OnGameplayLanguageChanged.RemoveDynamic(this, &ULoopInstructionWidget::RefreshInstruction);
	Super::NativeDestruct();
}

void ULoopInstructionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	RefreshInstruction(ULoopSettings::GetLoopSettings()->GameplayLanguage);
}

void ULoopInstructionWidget::SetInstruction(const FText& English, const FText& Chinese)
{
	EnglishInstruction = English;
	ChineseInstruction = Chinese;
	RefreshInstruction(ULoopSettings::GetLoopSettings()->GameplayLanguage);
}

void ULoopInstructionWidget::RefreshInstruction(ELoopGameplayLanguage Language)
{
	if (UTextBlock* Text = Cast<UTextBlock>(GetWidgetFromName(TEXT("Text_Instruction"))))
	{
		Text->SetText(Language == ELoopGameplayLanguage::Chinese && !ChineseInstruction.IsEmpty()
			? ChineseInstruction : EnglishInstruction);
	}
}
