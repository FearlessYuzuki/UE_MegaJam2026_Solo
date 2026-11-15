#include "MegaJam_2026/UI/LoopSettingsWidget.h"
#include "MegaJam_2026/Settings/LoopSettings.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "String/LexFromString.h"
#include "Engine/Font.h"
#include "UI/LoopMenuAudio.h"
#include "Misc/App.h"

namespace
{
	constexpr int32 FrameLimits[] = {30, 60, 90, 120, 144, 165, 240, 0};
}

void ULoopSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GetDefault<ULoopMenuAudio>()->Apply(WidgetTree);
	SetIsFocusable(true);
	SensitivitySlider = Cast<USlider>(GetWidgetFromName(TEXT("Slider_Sensitivity")));
	MusicSlider = Cast<USlider>(GetWidgetFromName(TEXT("Slider_Music")));
	if (MusicSlider) MusicSlider->OnValueChanged.AddUniqueDynamic(this, &ULoopSettingsWidget::HandleMusicSlider);
	SensitivityInput = Cast<UEditableTextBox>(GetWidgetFromName(TEXT("Input_Sensitivity")));
	QualityDropdown = Cast<UComboBoxString>(GetWidgetFromName(TEXT("Dropdown_Quality")));
	FrameLimitDropdown = Cast<UComboBoxString>(GetWidgetFromName(TEXT("Dropdown_FrameLimit")));
	LanguageDropdown = Cast<UComboBoxString>(GetWidgetFromName(TEXT("Dropdown_Language")));
	LumenStatus = Cast<UTextBlock>(GetWidgetFromName(TEXT("Text_LumenStatus")));
	SaveStatus = Cast<UTextBlock>(GetWidgetFromName(TEXT("Text_SaveStatus")));
	if (SensitivitySlider)
	{
		SensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &ULoopSettingsWidget::HandleSensitivitySlider);
	}
	if (SensitivityInput)
	{
		SensitivityInput->OnTextChanged.AddUniqueDynamic(this, &ULoopSettingsWidget::HandleSensitivityText);
		SensitivityInput->OnTextCommitted.AddUniqueDynamic(this, &ULoopSettingsWidget::HandleSensitivityCommitted);
	}
	if (QualityDropdown)
	{
		QualityDropdown->OnSelectionChanged.AddUniqueDynamic(this, &ULoopSettingsWidget::HandleQualityChanged);
	}
	if (FrameLimitDropdown)
	{
		FrameLimitDropdown->OnSelectionChanged.AddUniqueDynamic(this, &ULoopSettingsWidget::HandleOptionChanged);
	}
	if (LanguageDropdown)
	{
		LanguageDropdown->OnSelectionChanged.AddUniqueDynamic(this, &ULoopSettingsWidget::HandleLanguageChanged);
	}
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Apply"))))
	{
		Button->OnClicked.AddUniqueDynamic(this, &ULoopSettingsWidget::ApplySettings);
	}
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Back"))))
	{
		Button->OnClicked.AddUniqueDynamic(this, &ULoopSettingsWidget::CloseSettings);
	}
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Reset"))))
	{
		Button->OnClicked.AddUniqueDynamic(this, &ULoopSettingsWidget::ResetDefaults);
	}
	RefreshControls();
}

void ULoopSettingsWidget::OpenSettings()
{
	RefreshControls();
	SetVisibility(ESlateVisibility::Visible);
	SetKeyboardFocus();
}

void ULoopSettingsWidget::RefreshControls()
{
	const ULoopSettings* Settings = ULoopSettings::GetLoopSettings();
	bUpdatingControls = true;
	if (MusicSlider) MusicSlider->SetValue(Settings->MusicVolume);
	if (UTextBlock* Text = Cast<UTextBlock>(GetWidgetFromName(TEXT("Text_MusicValue"))))
		Text->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Settings->MusicVolume * 100.0f))));
	if (LanguageDropdown)
	{
		LanguageDropdown->SetSelectedIndex(static_cast<int32>(Settings->GameplayLanguage));
	}
	RefreshLanguage();
	if (SensitivitySlider)
	{
		SensitivitySlider->SetValue(Settings->MouseSensitivity);
	}
	if (SensitivityInput)
	{
		SensitivityInput->SetText(FText::FromString(FString::FromInt(Settings->MouseSensitivity)));
	}
	if (QualityDropdown)
	{
		QualityDropdown->SetSelectedIndex(Settings->GraphicsQuality);
	}
	if (FrameLimitDropdown)
	{
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(FrameLimits); ++Index)
		{
			if (FrameLimits[Index] == Settings->FrameRateLimit)
			{
				FrameLimitDropdown->SetSelectedIndex(Index);
				break;
			}
		}
	}
	bUpdatingControls = false;
	RefreshLumenStatus();
	SetSaveState(ESaveState::Empty);
}

bool ULoopSettingsWidget::IsChinese() const
{
	return LanguageDropdown ? LanguageDropdown->GetSelectedIndex() == 1
		: ULoopSettings::GetLoopSettings()->GameplayLanguage == ELoopGameplayLanguage::Chinese;
}

void ULoopSettingsWidget::RefreshLanguage()
{
	TGuardValue<bool> Guard(bUpdatingControls, true);
	const bool bChinese = IsChinese();
	struct FTranslation
	{
		const TCHAR* WidgetName;
		const TCHAR* English;
		const TCHAR* Chinese;
	};
	const FTranslation Translations[] = {
		{TEXT("Text_Title"), TEXT("Settings"), TEXT("设置")},
		{TEXT("Text_Eyebrow"), TEXT("LOOP  /  PREFERENCES"), TEXT("LOOP  /  偏好设置")},
		{TEXT("Text_Subtitle"), TEXT("Make yourself comfortable."), TEXT("调整设置，畅享游戏。")},
		{TEXT("Text_Sensitivity"), TEXT("Mouse sensitivity"), TEXT("鼠标灵敏度")},
		{TEXT("Text_Music"), TEXT("Music volume"), TEXT("音乐音量")},
		{TEXT("Hint_Music"), TEXT("Background music"), TEXT("背景音乐")},
		{TEXT("Hint_Sensitivity"), TEXT("1 - 200%  /  100% is the default"), TEXT("1 - 200%  /  默认值为 100%")},
		{TEXT("Text_Quality"), TEXT("Graphics quality"), TEXT("画质")},
		{TEXT("Hint_Quality"), TEXT("Lumen is available on High only"), TEXT("仅高画质启用 Lumen")},
		{TEXT("Text_FrameLimit"), TEXT("Frame rate limit"), TEXT("帧率限制")},
		{TEXT("Hint_FrameLimit"), TEXT("Choose your maximum frames per second"), TEXT("设置游戏的最高帧率")},
		{TEXT("Text_Language"), TEXT("Language"), TEXT("语言")},
		{TEXT("Hint_Language"), TEXT("Settings, narration and instructions"), TEXT("应用于设置页、游戏旁白和指示")},
		{TEXT("Label_Button_Back"), TEXT("Back"), TEXT("返回")},
		{TEXT("Label_Button_Reset"), TEXT("Reset defaults"), TEXT("恢复默认")},
		{TEXT("Label_Button_Apply"), TEXT("Apply settings"), TEXT("应用设置")}
	};
	UFont* LanguageFont = LoadObject<UFont>(nullptr, bChinese
		? TEXT("/Game/MegaJam2026/UI/Fonts/NotoSansCJKsc_Font.NotoSansCJKsc_Font")
		: TEXT("/Game/MegaJam2026/UI/Fonts/NotoSans_Font.NotoSans_Font"));
	for (const FTranslation& Translation : Translations)
	{
		if (UTextBlock* Text = Cast<UTextBlock>(GetWidgetFromName(Translation.WidgetName)))
		{
			Text->SetText(FText::FromString(bChinese ? Translation.Chinese : Translation.English));
			if (LanguageFont)
			{
				FSlateFontInfo Font = Text->GetFont();
				Font.FontObject = LanguageFont;
				Text->SetFont(Font);
			}
		}
	}
	for (UTextBlock* Text : {LumenStatus.Get(), SaveStatus.Get()})
	{
		if (Text && LanguageFont)
		{
			FSlateFontInfo Font = Text->GetFont();
			Font.FontObject = LanguageFont;
			Text->SetFont(Font);
		}
	}
	if (QualityDropdown)
	{
		const int32 QualityIndex = FMath::Clamp(QualityDropdown->GetSelectedIndex(), 0, 2);
		QualityDropdown->ClearOptions();
		for (const TCHAR* Option : {bChinese ? TEXT("低") : TEXT("Low"),
			bChinese ? TEXT("中") : TEXT("Medium"), bChinese ? TEXT("高") : TEXT("High")})
		{
			QualityDropdown->AddOption(Option);
		}
		QualityDropdown->SetSelectedIndex(QualityIndex);
	}
	if (FrameLimitDropdown)
	{
		const int32 FrameIndex = FMath::Clamp(FrameLimitDropdown->GetSelectedIndex(), 0, static_cast<int32>(UE_ARRAY_COUNT(FrameLimits)) - 1);
		FrameLimitDropdown->ClearOptions();
		for (const int32 FrameLimit : FrameLimits)
		{
			FrameLimitDropdown->AddOption(FrameLimit == 0 ? (bChinese ? TEXT("不限") : TEXT("Unlimited"))
				: FString::FromInt(FrameLimit));
		}
		FrameLimitDropdown->SetSelectedIndex(FrameIndex);
	}
	RefreshLumenStatus();
	SetSaveState(SaveState);
}

void ULoopSettingsWidget::RefreshLumenStatus()
{
	if (LumenStatus && QualityDropdown)
	{
		const bool bLumenEnabled = QualityDropdown->GetSelectedIndex() == 2;
		LumenStatus->SetText(FText::FromString(IsChinese()
			? (bLumenEnabled ? TEXT("LUMEN  /  开启") : TEXT("LUMEN  /  关闭"))
			: (bLumenEnabled ? TEXT("LUMEN  /  ON") : TEXT("LUMEN  /  OFF"))));
	}
}

void ULoopSettingsWidget::SetSaveState(ESaveState State)
{
	SaveState = State;
	if (SaveStatus)
	{
		const TCHAR* Text = State == ESaveState::Saved
			? (IsChinese() ? TEXT("设置已保存") : TEXT("Settings saved"))
			: (State == ESaveState::Unsaved ? (IsChinese() ? TEXT("有未保存的更改") : TEXT("Unsaved changes")) : TEXT(""));
		SaveStatus->SetText(FText::FromString(Text));
	}
}

void ULoopSettingsWidget::SetSensitivity(int32 Value, bool bUpdateInput)
{
	const int32 ClampedValue = FMath::Clamp(Value, 1, 200);
	bUpdatingControls = true;
	if (SensitivitySlider)
	{
		SensitivitySlider->SetValue(ClampedValue);
	}
	if (bUpdateInput && SensitivityInput)
	{
		SensitivityInput->SetText(FText::FromString(FString::FromInt(ClampedValue)));
	}
	bUpdatingControls = false;
	SetSaveState(ESaveState::Unsaved);
}

void ULoopSettingsWidget::HandleSensitivitySlider(float Value)
{
	if (!bUpdatingControls)
	{
		SetSensitivity(FMath::RoundToInt(Value), true);
	}
}

void ULoopSettingsWidget::HandleSensitivityText(const FText& Text)
{
	if (bUpdatingControls)
	{
		return;
	}
	FString Digits;
	for (const TCHAR Character : Text.ToString())
	{
		if (Character >= TEXT('0') && Character <= TEXT('9') && Digits.Len() < 3)
		{
			Digits.AppendChar(Character);
		}
	}
	if (Digits != Text.ToString() && SensitivityInput)
	{
		bUpdatingControls = true;
		SensitivityInput->SetText(FText::FromString(Digits));
		bUpdatingControls = false;
	}
	if (!Digits.IsEmpty())
	{
		int32 Value = 100;
		LexTryParseString(Value, *Digits);
		SetSensitivity(Value, false);
	}
}

void ULoopSettingsWidget::HandleSensitivityCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (!bUpdatingControls)
	{
		int32 Value = SensitivitySlider ? FMath::RoundToInt(SensitivitySlider->GetValue()) : 100;
		LexTryParseString(Value, *Text.ToString());
		SetSensitivity(Value, true);
	}
}

void ULoopSettingsWidget::HandleQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!bUpdatingControls)
	{
		RefreshLumenStatus();
	}
	HandleOptionChanged(SelectedItem, SelectionType);
}

void ULoopSettingsWidget::HandleOptionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!bUpdatingControls)
	{
		SetSaveState(ESaveState::Unsaved);
	}
}

void ULoopSettingsWidget::HandleLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!bUpdatingControls)
	{
		RefreshLanguage();
		SetSaveState(ESaveState::Unsaved);
	}
}

void ULoopSettingsWidget::ApplySettings()
{
	if (!SensitivityInput || !QualityDropdown || !FrameLimitDropdown || !LanguageDropdown)
	{
		return;
	}
	HandleSensitivityCommitted(SensitivityInput->GetText(), ETextCommit::OnEnter);
	int32 Sensitivity = 100;
	LexTryParseString(Sensitivity, *SensitivityInput->GetText().ToString());
	const int32 FrameIndex = FMath::Clamp(FrameLimitDropdown->GetSelectedIndex(), 0, static_cast<int32>(UE_ARRAY_COUNT(FrameLimits)) - 1);
	const int32 FrameLimit = FrameLimits[FrameIndex];
	ULoopSettings::GetLoopSettings()->CommitSettings(Sensitivity, QualityDropdown->GetSelectedIndex(), FrameLimit,
		static_cast<ELoopGameplayLanguage>(LanguageDropdown->GetSelectedIndex()));
	SetSaveState(ESaveState::Saved);
}

void ULoopSettingsWidget::CloseSettings()
{
	SetVisibility(ESlateVisibility::Collapsed);
	OnClosed.Broadcast();
}

void ULoopSettingsWidget::ResetDefaults()
{
	bUpdatingControls = true;
	if (QualityDropdown) QualityDropdown->SetSelectedIndex(2);
	if (FrameLimitDropdown) FrameLimitDropdown->SetSelectedOption(TEXT("120"));
	if (LanguageDropdown) LanguageDropdown->SetSelectedIndex(0);
	RefreshLanguage();
	bUpdatingControls = false;
	SetSensitivity(100, true);
	if (MusicSlider) MusicSlider->SetValue(1.0f);
	HandleQualityChanged(FString(), ESelectInfo::Direct);
}

void ULoopSettingsWidget::HandleMusicSlider(float Value)
{
	if (bUpdatingControls) return;
	ULoopSettings::GetLoopSettings()->SetMusicVolume(Value);
	if (UTextBlock* Text = Cast<UTextBlock>(GetWidgetFromName(TEXT("Text_MusicValue"))))
		Text->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.0f))));
}

void ULoopSettingsWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	for (const FName Name : {FName(TEXT("Back")), FName(TEXT("Apply")), FName(TEXT("Reset"))})
	{
		UButton* Button = Cast<UButton>(GetWidgetFromName(FName(*(TEXT("Button_") + Name.ToString()))));
		if (!Button) continue;
		float& Amount = ButtonHover.FindOrAdd(Name);
		Amount = FMath::FInterpConstantTo(Amount, Button->IsHovered() ? 1.0f : 0.0f, FApp::GetDeltaTime(), 5.0f);
		FButtonStyle Style = Button->GetStyle();
		const FLinearColor Tint(1, 1, 1, Amount);
		Style.Normal.TintColor = Tint;
		Style.Hovered.TintColor = Tint;
		Style.Pressed.TintColor = FLinearColor(1, 1, 1, 0.6f);
		Button->SetStyle(Style);
		if (UTextBlock* Label = Cast<UTextBlock>(GetWidgetFromName(FName(*(TEXT("Label_Button_") + Name.ToString())))))
			Label->SetColorAndOpacity(FMath::Lerp(FLinearColor::White, FLinearColor::Black, Amount));
	}
}

FReply ULoopSettingsWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	if (Event.GetKey() == EKeys::Escape)
	{
		CloseSettings();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(Geometry, Event);
}
