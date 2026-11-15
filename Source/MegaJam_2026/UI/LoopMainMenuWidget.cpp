#include "MegaJam_2026/UI/LoopMainMenuWidget.h"
#include "MegaJam_2026/UI/LoopSettingsWidget.h"
#include "Components/Button.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Core/LoopSessionSubsystem.h"
#include "UI/LoopMenuAudio.h"
#include "Engine/GameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/TextBlock.h"

ULoopMainMenuWidget* ULoopMainMenuWidget::ShowMainMenu(APlayerController* PlayerController, TSubclassOf<ULoopMainMenuWidget> MenuClass)
{
	if (!PlayerController || !MenuClass)
	{
		return nullptr;
	}
	ULoopMainMenuWidget* Menu = CreateWidget<ULoopMainMenuWidget>(PlayerController, MenuClass);
	if (Menu)
	{
		Menu->AddToViewport(200);
		PlayerController->SetShowMouseCursor(true);
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(Menu->TakeWidget());
		PlayerController->SetInputMode(InputMode);
	}
	return Menu;
}

void ULoopMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GetDefault<ULoopMenuAudio>()->Apply(WidgetTree);
	if (APlayerController* PlayerController = IsPauseMenu() ? nullptr : GetOwningPlayer())
	{
		for (TActorIterator<ACameraActor> Camera(GetWorld()); Camera; ++Camera)
		{
			if (Camera->ActorHasTag(OpeningCameraTag))
			{
				PreviousViewTarget = PlayerController->GetViewTarget();
				OpeningViewTarget = *Camera;
				PlayerController->SetViewTarget(*Camera);
				if (UWidget* Background = GetWidgetFromName(TEXT("MenuBackground")))
				{
					Background->SetVisibility(ESlateVisibility::Collapsed);
				}
				break;
			}
		}
	}
	if (!IsPauseMenu())
	{
		if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_NewGame")))) Button->OnClicked.AddUniqueDynamic(this, &ULoopMainMenuWidget::StartNewGame);
		if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Continue"))))
		{
			Button->OnClicked.AddUniqueDynamic(this, &ULoopMainMenuWidget::ContinueGame);
			Button->SetIsEnabled(GetGameInstance() && GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()->HasSavedGame());
		}
		if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Quit")))) Button->OnClicked.AddUniqueDynamic(this, &ULoopMainMenuWidget::QuitGame);
	}
	if (UButton* Button = Cast<UButton>(GetWidgetFromName(TEXT("Button_Settings"))))
	{
		Button->OnClicked.AddUniqueDynamic(this, &ULoopMainMenuWidget::OpenSettings);
	}
	if (ULoopSettingsWidget* Page = Cast<ULoopSettingsWidget>(GetWidgetFromName(TEXT("SettingsPage"))))
	{
		Page->OnClosed.AddUniqueDynamic(this, &ULoopMainMenuWidget::RestoreMenu);
	}
}

bool ULoopMainMenuWidget::IsSettingsOpen() const
{
	const UWidget* Settings = GetWidgetFromName(TEXT("SettingsPage"));
	return Settings && Settings->GetVisibility() != ESlateVisibility::Collapsed && Settings->GetVisibility() != ESlateVisibility::Hidden;
}

void ULoopMainMenuWidget::StartNewGame()
{
	GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()->NewGame();
}

void ULoopMainMenuWidget::ContinueGame()
{
	if (!GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()->ContinueGame())
	{
		if (UTextBlock* Status = Cast<UTextBlock>(GetWidgetFromName(TEXT("Text_MenuStatus")))) Status->SetText(FText::FromString(TEXT("No valid saved game found.")));
	}
}

void ULoopMainMenuWidget::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void ULoopMainMenuWidget::NativeDestruct()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		if (OpeningViewTarget.IsValid() && PlayerController->GetViewTarget() == OpeningViewTarget.Get()
			&& PreviousViewTarget.IsValid())
		{
			PlayerController->SetViewTarget(PreviousViewTarget.Get());
		}
	}
	OpeningViewTarget.Reset();
	PreviousViewTarget.Reset();
	Super::NativeDestruct();
}

void ULoopMainMenuWidget::OpenSettings()
{
	if (ULoopSettingsWidget* Page = Cast<ULoopSettingsWidget>(GetWidgetFromName(TEXT("SettingsPage"))))
	{
		if (UWidget* Menu = GetWidgetFromName(TEXT("MenuScale")))
		{
			Menu->SetRenderOpacity(0.16f);
			Menu->SetIsEnabled(false);
			Menu->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		Page->OpenSettings();
	}
}

void ULoopMainMenuWidget::RestoreMenu()
{
	if (UWidget* Menu = GetWidgetFromName(TEXT("MenuScale")))
	{
		Menu->SetVisibility(ESlateVisibility::Visible);
		Menu->SetRenderOpacity(1.0f);
		Menu->SetIsEnabled(true);
	}
	if (UWidget* Button = GetWidgetFromName(TEXT("Button_Settings")))
	{
		Button->SetKeyboardFocus();
	}
}
