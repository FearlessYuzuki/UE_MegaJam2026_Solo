#include "LoopPauseMenuWidget.h"

#include "Core/LoopSessionSubsystem.h"
#include "UI/LoopMenuAudio.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "InputCoreTypes.h"
#include "Components/RetainerBox.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/App.h"

void ULoopPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetIsFocusable(true);
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_NewGame")))) Button->OnClicked.AddUniqueDynamic(this, &ULoopPauseMenuWidget::Resume);
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Continue"))))
	{
		Button->OnClicked.AddUniqueDynamic(this, &ULoopPauseMenuWidget::SaveAndExit);
		GetDefault<ULoopMenuAudio>()->ApplyToButton(Button, true);
	}
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Quit"))))
	{
		Button->OnClicked.AddUniqueDynamic(this, &ULoopPauseMenuWidget::Restart);
		GetDefault<ULoopMenuAudio>()->ApplyToButton(Button, true);
	}
}

void ULoopPauseMenuWidget::Resume()
{
	GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()->ResumeGame();
}

void ULoopPauseMenuWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	const float RealDelta = FMath::Clamp(static_cast<float>(FApp::GetDeltaTime()), 0.0f, 0.1f);
	for (const FString& Name : {FString(TEXT("NewGame")), FString(TEXT("Settings")), FString(TEXT("Continue")), FString(TEXT("Quit"))})
	{
		UButton* Button = Cast<UButton>(GetWidgetFromName(FName(*(TEXT("Button_") + Name))));
		URetainerBox* Sweep = Cast<URetainerBox>(GetWidgetFromName(FName(*(TEXT("Sweep_") + Name))));
		if (!Button || !Sweep) continue;
		float& Phase = HoverPhases.FindOrAdd(FName(*Name));
		const float Target = !IsSettingsOpen() && Button->IsHovered() ? 1.0f : (Phase > 0.0f ? 2.0f : 0.0f);
		Phase = FMath::FInterpConstantTo(Phase, Target, RealDelta, 5.0f);
		if (Phase >= 2.0f) Phase = 0.0f;
		if (UMaterialInstanceDynamic* Material = Sweep->GetEffectMaterial()) Material->SetScalarParameterValue(TEXT("SweepPhase"), Phase);
		Sweep->RequestRender();
	}
}

void ULoopPauseMenuWidget::SaveAndExit()
{
	if (!GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()->SaveAndExit())
	{
		if (UTextBlock* Status = Cast<UTextBlock>(GetWidgetFromName(TEXT("Text_MenuStatus")))) Status->SetText(FText::FromString(TEXT("Could not save. Your game remains open.")));
	}
}

void ULoopPauseMenuWidget::Restart()
{
	GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()->RestartLevel();
}

FReply ULoopPauseMenuWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape && !IsSettingsOpen())
	{
		Resume();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(Geometry, Event);
}
