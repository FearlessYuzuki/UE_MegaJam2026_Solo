#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PlayerCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UInputAction;
class UFootstepAudioComponent;
class ULoopSaveGame;
struct FInputActionValue;

UCLASS()
class MEGAJAM_2026_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> FirstPersonSpringArm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<UCameraComponent> DebugCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = true))
	TObjectPtr<USpringArmComponent> DebugSpringArm;

public:
	void PrepareForMenu();
	void RestoreSavedPlayer(const ULoopSaveGame& Save);
	FName GetSavedSpawnPoint() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UFootstepAudioComponent> FootstepAudio;

	/*virtual void SwitchToFirstCam();
	virtual void SwitchToThirdCam();*/

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual FRotator GetViewRotation() const override;
	virtual void OnJumped_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tools|DevMode")
	bool bDevMode = false;

	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tools|AutoBhoping")
	bool AutoBhopFunction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Mantle")
	bool bEnableMantling = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Mantle",
		meta = (ClampMin = "0.0", Units = "s"))
	float MantleHoldTime = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tools")
	bool SpeedShowingSwitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tools")
	float PlayerDeathHeight = 0.f;

protected:
	void MoveInput(const FInputActionValue& Value);
	void MoveInputReleased();
	void ClearSourceMovementInput();
	void MouseLookInput(const FInputActionValue& Value);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputMappingContext> IMC_Choice;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = true))
	TObjectPtr<UInputAction> CrouchAction;
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Domove(float Right, float Left);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouch();
	
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoUnCrouch();
	
	virtual void ShowVelocity();

protected:
	void Respawn();

	UPROPERTY()
	TObjectPtr<AActor> CurrentSpawnPoint;

public:
	UFUNCTION(BlueprintCallable)
	void SetSpawnPoint(AActor* InPoint);

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable, Category = "Camera|Gravity")
	void SetCameraAngle();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Gravity",
		meta = (ClampMin = "0.0", Units = "s"))
	float CameraFlipDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Crouch",
		meta = (ClampMin = "0.0", Units = "s"))
	float CrouchCameraTransitionDuration = 0.2f;

	void BeginCrouchCameraTransition(const FVector& CapsuleLocationCompensation);

private:
	void DoJumpHeld();
	void UpdateJumpInput(float DeltaTime);
	void ResetJumpInputState();
	void CancelMantle();
	bool IsMantling() const;

	bool bJumpInputHeld = false;
	bool bMantleConsumedForHold = false;
	float JumpHeldTime = 0.0f;

	void CenterFirstPersonCamera();
	void UpdateCrouchCamera(float DeltaTime);
	void ResetCrouchCamera();
	FVector CrouchCameraStartOffset = FVector::ZeroVector;
	float CrouchCameraElapsed = 0.0f;

	void UpdateCameraAngle(float DeltaTime);
	void SetCameraFlipInputLocked(bool bLocked);
	float GetCameraInputDirection() const;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Camera|Gravity",
		meta = (AllowPrivateAccess = "true"))
	bool bIsCameraFlipping = false;

	TWeakObjectPtr<AController> CameraFlipInputController;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Camera|Gravity",
		meta = (AllowPrivateAccess = "true"))
	float CurrentCameraRoll = 0.0f;

	float StartCameraRoll = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Transient, Category = "Camera|Gravity",
		meta = (AllowPrivateAccess = "true"))
	float TargetCameraRoll = 0.0f;

	float CameraFlipElapsed = 0.0f;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ShowFPS|Tools")
	bool bShowFPS = true;

	UFUNCTION(BlueprintCallable, Category = "ShowFPS|Tools")
	void showFramepos(bool bShouldShowFPS);

private:
	double FrameStatsStartTime = 0.0;
	double FrameStatsLastFrameTime = 0.0;
	double FrameStatsPeakFrameTime = 0.0;
	uint32 FrameStatsFrameCount = 0;
};
