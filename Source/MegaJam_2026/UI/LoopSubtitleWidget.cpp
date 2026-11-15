#include "LoopSubtitleWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"

ULoopSubtitleWidget::ULoopSubtitleWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Game/MegaJam2026/UI/Fonts/NotoSansCJKsc_Font.NotoSansCJKsc_Font"));
	SubtitleFont = Font.Object;
}

void ULoopSubtitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("SubtitleRoot"));
	WidgetTree->RootWidget = Root;
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("SubtitleBackground"));
	Background->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f));
	Background->SetPadding(FMargin(28.0f, 14.0f));
	UCanvasPanelSlot* BackgroundSlot = Root->AddChildToCanvas(Background);
	BackgroundSlot->SetAnchors(FAnchors(0.16f, 0.88f, 0.84f, 0.88f));
	BackgroundSlot->SetAlignment(FVector2D(0.0f, 1.0f));
	BackgroundSlot->SetOffsets(FMargin(0.0f));
	BackgroundSlot->SetAutoSize(true);
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Text_Instruction"));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Text->SetFont(FSlateFontInfo(SubtitleFont, 26, FName(TEXT("Default"))));
	Text->SetJustification(ETextJustify::Center);
	Text->SetAutoWrapText(true);
	Text->SetMargin(FMargin(0.0f));
	Background->SetHorizontalAlignment(HAlign_Fill);
	Background->SetVerticalAlignment(VAlign_Center);
	Background->SetContent(Text);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}
