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
class UUserWidget;
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
	virtual void SwitchToFirstCam();
	virtual void SwitchToThirdCam();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tools|DevMode")
	bool bDevMode = false;

	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tools|AutoBhoping")
	bool AutoBhopFunction = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tools")
	bool SpeedShowingSwitch = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tools")
	float PlayerDeathHeight = 0.f;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DebugUI")
	TSubclassOf<class UUserWidget> DebugUI;

	UPROPERTY()
	TObjectPtr<UUserWidget> DebugWidgetInstance;

protected:
	void MoveInput(const FInputActionValue& Value);
	void MoveInputReleased();
	void ClearSourceMovementInput();
	void MouseLookInput(const FInputActionValue& Value);
	void DebugMenuCalled(const FInputActionValue& Value);

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
	TObjectPtr<UInputAction> DebugAction;

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void Domove(float Right, float Left);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

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
};
