#include "MusicPlaylistActor.h"

#include "Components/AudioComponent.h"
#include "Settings/LoopSettings.h"

AMusicPlaylistActor::AMusicPlaylistActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	RootComponent = Audio;
	Audio->bAutoActivate = false;
	Audio->bAllowSpatialization = false;
	Audio->OnAudioFinished.AddDynamic(this, &AMusicPlaylistActor::HandleTrackFinished);
}

void AMusicPlaylistActor::BeginPlay()
{
	Super::BeginPlay();
	if (bStartOnBeginPlay)
	{
		StartPlaylist();
	}
}

void AMusicPlaylistActor::StartPlaylist()
{
	if (Audio->IsPlaying())
	{
		return;
	}
	if (Playlist.Num() == 0)
	{
		CurrentTrackIndex = INDEX_NONE;
		return;
	}
	bPlaylistPlaying = true;
	PlayTrack(CurrentTrackIndex == INDEX_NONE ? 0 : CurrentTrackIndex);
}

void AMusicPlaylistActor::StopPlaylist()
{
	bPlaylistPlaying = false;
	Audio->Stop();
	CurrentTrackIndex = INDEX_NONE;
}

void AMusicPlaylistActor::PlayTrack(int32 Index)
{
	if (!Playlist.IsValidIndex(Index) || !Playlist[Index])
	{
		StopPlaylist();
		return;
	}
	CurrentTrackIndex = Index;
	Audio->SetSound(Playlist[Index]);
	RefreshVolume();
	Audio->Play();
}

void AMusicPlaylistActor::HandleTrackFinished()
{
	if (!bPlaylistPlaying || Playlist.Num() == 0)
	{
		return;
	}
	const int32 NextIndex = CurrentTrackIndex + 1;
	if (NextIndex >= Playlist.Num())
	{
		if (!bLoopPlaylist)
		{
			CurrentTrackIndex = INDEX_NONE;
			return;
		}
		PlayTrack(0);
		return;
	}
	PlayTrack(NextIndex);
}

void AMusicPlaylistActor::RefreshVolume()
{
	const bool bMusic = Playlist.ContainsByPredicate([](const USoundBase* Sound) { return Sound && Sound->GetPathName().Contains(TEXT("/InGameBGM/")); });
	Audio->SetVolumeMultiplier(FMath::Max(Volume, 0.0f) * (bMusic ? ULoopSettings::GetLoopSettings()->MusicVolume : 1.0f));
}
