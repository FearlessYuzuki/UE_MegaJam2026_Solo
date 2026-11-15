#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialOpeningSequence.generated.h"

class AGroundBlock;
class APressurePlateLift;
class APressurePlateLaunch;
class APressurePlateGravity;
class APressurePlateGoal;
class ALoopNarrationDirector;

UCLASS(Blueprintable)
class MEGAJAM_2026_API ATutorialOpeningSequence : public AActor
{
	GENERATED_BODY()

public:
	ATutorialOpeningSequence();
	virtual void Tick(float DeltaSeconds) override;
	void RestoreSavedState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial") TObjectPtr<APressurePlateLift> OpeningPlate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial") TArray<TObjectPtr<AGroundBlock>> RemainingBlocks;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial") TArray<TObjectPtr<AGroundBlock>> IntroBlocks;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial|Recovery") TArray<TObjectPtr<AGroundBlock>> InvertedBlocks;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial|Recovery", meta=(ClampMin="0.0", Units="cm")) float RecoveryFallDistance = 250.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial") TObjectPtr<APressurePlateLaunch> LaunchPlate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial") TObjectPtr<APressurePlateGravity> GravityPlate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial") TObjectPtr<APressurePlateGoal> CompletionPlate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial") TObjectPtr<ALoopNarrationDirector> NarrationDirector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial", meta=(ClampMin="0.0")) float LandingDelay = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial", meta=(ClampMin="0.0")) float PlateRiseHeight = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tutorial", meta=(ClampMin="0.01")) float PlateRiseDuration = 2.0f;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION() void HandlePlatePressed(AActor* PressingActor);
	UFUNCTION() void HandleFirstSurface(AActor* ActivatingActor);
	UFUNCTION() void HandleLastIntroSurface(AActor* ActivatingActor);
	UFUNCTION() void HandleLaunch(AActor* LaunchedActor, FVector LaunchVelocity);
	UFUNCTION() void HandleFlip(AActor* PlayerActor, FVector GravityDirection);
	UFUNCTION() void HandleGoal(FName GoalTag);
	UFUNCTION() void HandleSectionFinished(FName Section);
	void QueueNarration(FName Section);
	void ApplyGoalState();
	void UpdateInvertedRecovery();
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Tutorial|Recovery", meta=(AllowPrivateAccess="true")) FName InvertedCheckpoint;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Tutorial|Progress", meta=(AllowPrivateAccess="true")) bool bLanded = false;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Tutorial|Progress", meta=(AllowPrivateAccess="true")) bool bPlateReady = false;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Tutorial|Progress", meta=(AllowPrivateAccess="true")) bool bRemainingRaised = false;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Tutorial|Progress", meta=(AllowPrivateAccess="true")) float TimeSinceLanding = 0.0f;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Tutorial|Progress", meta=(AllowPrivateAccess="true")) float PlateRiseElapsed = 0.0f;
	UPROPERTY(SaveGame) FVector PlateTargetLocation;
	UPROPERTY(SaveGame) bool bIntroBlocksRequested = false;
	UPROPERTY(SaveGame) bool bGroundHintPlayed = false;
	UPROPERTY(SaveGame) bool bPlateHintPlayed = false;
	UPROPERTY(SaveGame) bool bLaunchHintPlayed = false;
	UPROPERTY(SaveGame) bool bFlipHintPlayed = false;
	UPROPERTY(SaveGame) bool bExitHintPlayed = false;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Tutorial|Progress", meta=(AllowPrivateAccess="true")) bool bGoalArmed = false;
	void ApplyPlateState();
};
