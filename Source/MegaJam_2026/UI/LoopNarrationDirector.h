#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "LoopNarrationDirector.generated.h"

USTRUCT(BlueprintType)
struct FLoopNarrationRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Section;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FText Chinese;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FText English;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5"))
	float Duration = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TriggerNote;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoopNarrationSectionFinished, FName, Section);

UCLASS()
class MEGAJAM_2026_API ALoopNarrationDirector : public AActor
{
	GENERATED_BODY()

public:
	ALoopNarrationDirector();

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void PlaySection(FName Section);

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void QueueSection(FName Section);
	void CaptureSavedState();
	void RestoreSavedState();

	UFUNCTION(BlueprintCallable, Category = "Narration")
	void StopNarration();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narration")
	TObjectPtr<UDataTable> Script;

	UPROPERTY(BlueprintAssignable, Category = "Narration")
	FLoopNarrationSectionFinished OnSectionFinished;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void LoadSection(FName Section);
	void ShowNextLine();

	UPROPERTY(Transient)
	TArray<FLoopNarrationRow> Lines;

	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Narration", meta=(AllowPrivateAccess="true")) FName CurrentSection;
	UPROPERTY(SaveGame, VisibleInstanceOnly, BlueprintReadOnly, Category="Narration", meta=(AllowPrivateAccess="true")) int32 CurrentLineIndex = 0;
	UPROPERTY(SaveGame) TArray<FName> PendingSections;
	UPROPERTY(SaveGame) float SavedLineRemaining = 0.0f;
	FTimerHandle LineTimer;
};
