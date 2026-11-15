#include "MegaJam_2026/Core/MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Core/LoopSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"
#include "Components/InputComponent.h"
#include "UI/LoopMainMenuWidget.h"
#include "UI/LoopPauseMenuWidget.h"
#include "Engine/Engine.h"

AMyPlayerController::AMyPlayerController()
{
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (GEngine)
	{
		GEngine->bEnableOnScreenDebugMessages = false;
		GEngine->bEnableOnScreenDebugMessagesDisplay = false;
	}
	if (!MainMenuClass) MainMenuClass = LoadClass<ULoopMainMenuWidget>(nullptr, TEXT("/Game/MegaJam2026/UI/Menus/WBP_MainMenuSplitExample.WBP_MainMenuSplitExample_C"));
	if (!PauseMenuClass) PauseMenuClass = LoadClass<ULoopPauseMenuWidget>(nullptr, TEXT("/Game/MegaJam2026/UI/Menus/WBP_PauseMenuUI.WBP_PauseMenuUI_C"));
	if (IsLocalController())
	{
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (ULoopSessionSubsystem* Session = GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()) Session->InitializeController(this);
		}));
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AMyPlayerController::TogglePauseMenu).bExecuteWhenPaused = true;

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : InputMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AMyPlayerController::TogglePauseMenu()
{
	if (ULoopSessionSubsystem* Session = GetGameInstance()->GetSubsystem<ULoopSessionSubsystem>()) Session->TogglePauseMenu();
}
