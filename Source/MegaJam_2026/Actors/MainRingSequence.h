#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MainRingSequence.generated.h"

class AGroundBlock;
class APressurePlateGravity;
class APressurePlateLaunch;
class APressurePlateGoal;
class ALoopNarrationDirector;
class ACameraActor;
class UMediaPlayer;
class ULoopEndingWidget;
class UMeshComponent;
class UExponentialHeightFogComponent;

USTRUCT(BlueprintType)
struct FMainRingStage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<AGroundBlock> LandingBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<APressurePlateGravity> ExitPlate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<TObjectPtr<AGroundBlock>> RevealBlocks;
};

UCLASS(Blueprintable)
class MEGAJAM_2026_API AMainRingSequence : public AActor
{
	GENERATED_BODY()

public:
	AMainRingSequence();
	virtual void Tick(float DeltaSeconds) override;
	void RestoreSavedState();
	UFUNCTION(BlueprintPure, Category="Ring|Ending") bool IsEndingPlaying() const { return EndingPhase != 0; }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring|Ending") FVector RingCenter = FVector(1600, 21156, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring|Ending") float OverviewHeight = 65000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring") TArray<FMainRingStage> Stages;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring") TArray<TObjectPtr<AGroundBlock>> CourseBlocks;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring") TObjectPtr<AGroundBlock> OpeningBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring") TObjectPtr<AGroundBlock> FinaleBlock;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring") TObjectPtr<APressurePlateLaunch> FinaleLaunch;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring") TObjectPtr<APressurePlateGoal> CompletionPlate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring") TObjectPtr<ALoopNarrationDirector> NarrationDirector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ring|Recovery", meta=(ClampMin="100.0", Units="cm")) float RecoveryFallDistance = 450.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	UFUNCTION() void HandleFinaleLaunch(AActor* LaunchedActor, FVector LaunchVelocity);
	UFUNCTION() void HandleSectionFinished(FName Section);
	void ApplyGateState();
	void UpdateRecovery();
	UFUNCTION() void HandleCompletion(FName GoalTag);
	UFUNCTION() void HandleMovieFinished();
	void TickEnding(float DeltaSeconds);
	void EnterEndingPhase(int32 Phase);
	void ReleasePlayer();
	void SetOverviewVisibility(bool bOverview);
	FVector ExplorationGravity = FVector::DownVector;
	UPROPERTY(Transient) TObjectPtr<ACameraActor> EndingCamera;
	UPROPERTY(Transient) TObjectPtr<UMediaPlayer> EndingMedia;
	UPROPERTY(Transient) TObjectPtr<ULoopEndingWidget> EndingUI;
	TArray<TWeakObjectPtr<UMeshComponent>> HiddenWater;
	TArray<TPair<TWeakObjectPtr<UExponentialHeightFogComponent>, float>> FogDensities;
	FVector CameraStart;
	FRotator CameraStartRotation;
	int32 EndingPhase = 0;
	float EndingElapsed = 0.0f;
	bool bMovieFinished = false;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Ring|Progress", meta=(AllowPrivateAccess="true")) bool bCelebrationCompleted = false;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Ring|Progress", meta=(AllowPrivateAccess="true")) int32 CompletedFlips = 0;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Ring|Progress", meta=(AllowPrivateAccess="true")) FName CheckpointBlock;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Ring|Progress", meta=(AllowPrivateAccess="true")) bool bFinaleStarted = false;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Ring|Progress", meta=(AllowPrivateAccess="true")) bool bEndingFinished = false;
	UPROPERTY(SaveGame) bool bLandingNarrationStarted = false;
};
