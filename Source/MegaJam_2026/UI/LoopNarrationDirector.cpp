#include "LoopNarrationDirector.h"

#include "MegaJam_2026/UI/LoopSubtitleSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

ALoopNarrationDirector::ALoopNarrationDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALoopNarrationDirector::PlaySection(FName Section)
{
	StopNarration();
	LoadSection(Section);
	ShowNextLine();
}

void ALoopNarrationDirector::QueueSection(FName Section)
{
	if (Section.IsNone() || CurrentSection == Section || PendingSections.Contains(Section)) return;
	if (CurrentSection.IsNone())
	{
		PlaySection(Section);
	}
	else
	{
		PendingSections.Add(Section);
	}
}

void ALoopNarrationDirector::LoadSection(FName Section)
{
	Lines.Reset();
	CurrentLineIndex = 0;
	if (!Script || !Script->GetRowStruct() || !Script->GetRowStruct()->IsChildOf(FLoopNarrationRow::StaticStruct()))
	{
		return;
	}
	for (const TPair<FName, uint8*>& Row : Script->GetRowMap())
	{
		const FLoopNarrationRow* Line = reinterpret_cast<const FLoopNarrationRow*>(Row.Value);
		if (Line->Section == Section)
		{
			Lines.Add(*Line);
		}
	}
	Lines.Sort([](const FLoopNarrationRow& First, const FLoopNarrationRow& Second)
	{
		return First.Sequence < Second.Sequence;
	});
	CurrentSection = Section;
}

void ALoopNarrationDirector::ShowNextLine()
{
	UGameInstance* GameInstance = GetGameInstance();
	ULoopSubtitleSubsystem* Subtitles = GameInstance ? GameInstance->GetSubsystem<ULoopSubtitleSubsystem>() : nullptr;
	if (!Subtitles)
	{
		StopNarration();
		return;
	}
	if (!Lines.IsValidIndex(CurrentLineIndex))
	{
		const FName FinishedSection = CurrentSection;
		Subtitles->ClearSubtitle();
		Lines.Reset();
		CurrentLineIndex = 0;
		CurrentSection = NAME_None;
		if (!FinishedSection.IsNone())
		{
			OnSectionFinished.Broadcast(FinishedSection);
		}
		if (CurrentSection.IsNone() && !PendingSections.IsEmpty())
		{
			const FName NextSection = PendingSections[0];
			PendingSections.RemoveAt(0);
			LoadSection(NextSection);
			ShowNextLine();
		}
		return;
	}
	const FLoopNarrationRow& Line = Lines[CurrentLineIndex++];
	Subtitles->ShowSubtitle(Line.English, Line.Chinese);
	GetWorldTimerManager().SetTimer(LineTimer, this, &ALoopNarrationDirector::ShowNextLine, FMath::Max(Line.Duration, 0.5f), false);
}

void ALoopNarrationDirector::StopNarration()
{
	GetWorldTimerManager().ClearTimer(LineTimer);
	Lines.Reset();
	CurrentLineIndex = 0;
	CurrentSection = NAME_None;
	PendingSections.Reset();
	SavedLineRemaining = 0.0f;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULoopSubtitleSubsystem* Subtitles = GameInstance->GetSubsystem<ULoopSubtitleSubsystem>())
		{
			Subtitles->ClearSubtitle();
		}
	}
}

void ALoopNarrationDirector::CaptureSavedState()
{
	SavedLineRemaining = GetWorldTimerManager().GetTimerRemaining(LineTimer);
}

void ALoopNarrationDirector::RestoreSavedState()
{
	GetWorldTimerManager().ClearTimer(LineTimer);
	if (CurrentSection.IsNone()) return;
	const int32 SavedIndex = CurrentLineIndex;
	LoadSection(CurrentSection);
	CurrentLineIndex = SavedIndex;
	if (Lines.IsValidIndex(CurrentLineIndex - 1))
	{
		const FLoopNarrationRow& Line = Lines[CurrentLineIndex - 1];
		GetGameInstance()->GetSubsystem<ULoopSubtitleSubsystem>()->ShowSubtitle(Line.English, Line.Chinese);
		GetWorldTimerManager().SetTimer(LineTimer, this, &ALoopNarrationDirector::ShowNextLine, FMath::Max(SavedLineRemaining, 0.1f), false);
	}
	else
	{
		ShowNextLine();
	}
}

void ALoopNarrationDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopNarration();
	Super::EndPlay(EndPlayReason);
}
