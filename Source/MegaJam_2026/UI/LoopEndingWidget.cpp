#include "UI/LoopEndingWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Font.h"
#include "Materials/MaterialInterface.h"
#include "Settings/LoopSettings.h"

void ULoopEndingWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>();
	WidgetTree->RootWidget = Root;
	Movie = WidgetTree->ConstructWidget<UImage>();
	Movie->SetBrushFromMaterial(LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/MegaJam2026/UI/Media/M_UI_LoopIntro.M_UI_LoopIntro")));
	Black = WidgetTree->ConstructWidget<UImage>();
	Black->SetColorAndOpacity(FLinearColor::Black);
	for (UImage* Image : {Movie.Get(), Black.Get()})
	{
		UOverlaySlot* ImageSlot = Root->AddChildToOverlay(Image);
		ImageSlot->SetHorizontalAlignment(HAlign_Fill);
		ImageSlot->SetVerticalAlignment(VAlign_Fill);
	}
	Thanks = WidgetTree->ConstructWidget<UTextBlock>();
	Thanks->SetText(FText::FromString(ULoopSettings::GetLoopSettings()->GameplayLanguage == ELoopGameplayLanguage::Chinese ? TEXT("感谢游玩") : TEXT("Thank you for playing")));
	Thanks->SetFont(FSlateFontInfo(LoadObject<UFont>(nullptr, TEXT("/Game/MegaJam2026/UI/Fonts/NotoSansCJKsc_Font.NotoSansCJKsc_Font")), 42));
	Thanks->SetColorAndOpacity(FLinearColor::White);
	UOverlaySlot* TextSlot = Root->AddChildToOverlay(Thanks);
	TextSlot->SetHorizontalAlignment(HAlign_Center);
	TextSlot->SetVerticalAlignment(VAlign_Center);
	SetVisibility(ESlateVisibility::HitTestInvisible);
	SetPresentation(0, 0, 0);
}

void ULoopEndingWidget::SetPresentation(float BlackOpacity, float MovieOpacity, float TextOpacity)
{
	if (Black) Black->SetRenderOpacity(BlackOpacity);
	if (Movie) Movie->SetRenderOpacity(MovieOpacity);
	if (Thanks) Thanks->SetRenderOpacity(TextOpacity);
}
