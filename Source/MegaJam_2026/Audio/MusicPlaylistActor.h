#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MusicPlaylistActor.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS()
class MEGAJAM_2026_API AMusicPlaylistActor : public AActor
{
	GENERATED_BODY()

public:
	AMusicPlaylistActor();

	UFUNCTION(BlueprintCallable, Category = "Audio|Playlist")
	void StartPlaylist();

	UFUNCTION(BlueprintCallable, Category = "Audio|Playlist")
	void StopPlaylist();

	UFUNCTION(BlueprintCallable, Category = "Audio|Playlist")
	void RefreshVolume();

	UFUNCTION(BlueprintPure, Category = "Audio|Playlist")
	int32 GetCurrentTrackIndex() const { return CurrentTrackIndex; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleTrackFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Playlist")
	TObjectPtr<UAudioComponent> Audio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Playlist")
	TArray<TObjectPtr<USoundBase>> Playlist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Playlist", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Volume = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Playlist")
	bool bStartOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Playlist")
	bool bLoopPlaylist = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Audio|Playlist")
	int32 CurrentTrackIndex = INDEX_NONE;

private:
	void PlayTrack(int32 Index);
	bool bPlaylistPlaying = false;
};
