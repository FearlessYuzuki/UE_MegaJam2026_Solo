#include "LoopSubtitleSubsystem.h"

#include "MegaJam_2026/UI/LoopSubtitleWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void ULoopSubtitleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ULoopSubtitleSubsystem::HandleWorldCleanup);
}

void ULoopSubtitleSubsystem::HandleWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if (TimerWorld.Get() == World)
	{
		ClearSubtitle();
	}
}

void ULoopSubtitleSubsystem::ShowSubtitle(const FText& English, const FText& Chinese, float Duration)
{
	if (English.IsEmpty() && Chinese.IsEmpty())
	{
		ClearSubtitle();
		return;
	}
	if (TimerWorld.IsValid())
	{
		TimerWorld->GetTimerManager().ClearTimer(SubtitleTimer);
	}
	APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
	if (!PlayerController)
	{
		return;
	}
	if (!Subtitle || Subtitle->GetWorld() != PlayerController->GetWorld())
	{
		ClearSubtitle();
		Subtitle = CreateWidget<ULoopSubtitleWidget>(PlayerController, ULoopSubtitleWidget::StaticClass());
	}
	if (!Subtitle)
	{
		return;
	}
	Subtitle->SetInstruction(English, Chinese);
	if (!Subtitle->IsInViewport())
	{
		Subtitle->AddToViewport(50);
	}
	TimerWorld = PlayerController->GetWorld();
	if (Duration > 0.0f)
	{
		TimerWorld->GetTimerManager().SetTimer(SubtitleTimer, this, &ULoopSubtitleSubsystem::ClearSubtitle, Duration, false);
	}
}

void ULoopSubtitleSubsystem::ClearSubtitle()
{
	if (TimerWorld.IsValid())
	{
		TimerWorld->GetTimerManager().ClearTimer(SubtitleTimer);
	}
	TimerWorld.Reset();
	if (Subtitle)
	{
		Subtitle->RemoveFromParent();
		Subtitle = nullptr;
	}
}

void ULoopSubtitleSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
	ClearSubtitle();
	Super::Deinitialize();
}
