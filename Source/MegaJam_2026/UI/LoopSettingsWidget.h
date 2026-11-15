#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "LoopSettingsWidget.generated.h"

class USlider;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLoopSettingsClosed);

UCLASS()
class MEGAJAM_2026_API ULoopSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Settings")
	FLoopSettingsClosed OnClosed;

	UFUNCTION(BlueprintCallable, Category="Settings")
	void OpenSettings();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USlider> SensitivitySlider;
	UPROPERTY(Transient) TObjectPtr<USlider> MusicSlider;
	UFUNCTION() void HandleMusicSlider(float Value);
	TMap<FName, float> ButtonHover;
	UPROPERTY(Transient)
	TObjectPtr<UEditableTextBox> SensitivityInput;
	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> QualityDropdown;
	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> FrameLimitDropdown;
	UPROPERTY(Transient)
	TObjectPtr<UComboBoxString> LanguageDropdown;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> LumenStatus;
	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> SaveStatus;

	bool bUpdatingControls = false;
	enum class ESaveState : uint8 { Empty, Unsaved, Saved };
	ESaveState SaveState = ESaveState::Empty;
	bool IsChinese() const;
	void RefreshLanguage();
	void RefreshLumenStatus();
	void SetSaveState(ESaveState State);
	void RefreshControls();
	void SetSensitivity(int32 Value, bool bUpdateInput);

	UFUNCTION()
	void HandleSensitivitySlider(float Value);
	UFUNCTION()
	void HandleSensitivityText(const FText& Text);
	UFUNCTION()
	void HandleSensitivityCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UFUNCTION()
	void HandleQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void HandleOptionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void HandleLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void ApplySettings();
	UFUNCTION()
	void CloseSettings();
	UFUNCTION()
	void ResetDefaults();
};
