#include "MegaJam_2026/Characters/PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "InputActionValue.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MegaJam_2026/Characters/SourceCharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USourceCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->SetCapsuleHalfHeight(160.0f, false);

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCamera"));
	FirstPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("FirstPersonSpringArm"));

	FirstPersonSpringArm->SetupAttachment(RootComponent);
	FirstPersonCamera->SetupAttachment(FirstPersonSpringArm, USpringArmComponent::SocketName);

	CenterFirstPersonCamera();
	FirstPersonCamera->FieldOfView = 118.0f;
	FirstPersonSpringArm->bUsePawnControlRotation = false;
	FirstPersonCamera->bUsePawnControlRotation = true;

	DebugCamera = CreateDefaultSubobject<UCameraComponent>(FName("DebugCamCamera"));
	DebugSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("Debug Spring Arm"));

	DebugCamera->bUsePawnControlRotation = false;
	DebugSpringArm->bUsePawnControlRotation = false;
	DebugSpringArm->SetupAttachment(RootComponent);
	DebugCamera->SetupAttachment(DebugSpringArm, USpringArmComponent::SocketName);
	DebugSpringArm->AddWorldRotation(FRotator(-90.0f, 0.0f, 0.0f));
	DebugCamera->AddWorldRotation(FRotator(0.0f, 0.0f, 0.0f));

	DebugCamera->FieldOfView = 102.f;
	DebugSpringArm->TargetArmLength = 1600.f;

	if (bDevMode == false)
	{
		FirstPersonCamera->Activate();
		DebugCamera->Deactivate();
	}
	
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	CenterFirstPersonCamera();
}

void APlayerCharacter::CenterFirstPersonCamera()
{
	if (!FirstPersonSpringArm || !FirstPersonCamera)
	{
		return;
	}

	FirstPersonSpringArm->TargetArmLength = 0.0f;
	FirstPersonSpringArm->SocketOffset = FVector::ZeroVector;
	FirstPersonSpringArm->TargetOffset = FVector::ZeroVector;
	FirstPersonSpringArm->SetRelativeLocation(FVector::ZeroVector);
	FirstPersonCamera->SetRelativeLocation(FVector::ZeroVector);
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SetCameraFlipInputLocked(false);
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCameraAngle(DeltaTime);
	if (SpeedShowingSwitch == true)
	{
		ShowVelocity();
	}

	AWorldSettings* p_worldSettings = GetWorld()->GetWorldSettings();
	if (p_worldSettings)
	{
		float f_killZheight = p_worldSettings->KillZ;
		if (GetActorLocation().Z < f_killZheight)
		{
			Respawn();
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::MouseLookInput);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::MoveInput);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this,
		                                   &APlayerCharacter::MoveInputReleased);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this,
		                                   &APlayerCharacter::MoveInputReleased);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this,&APlayerCharacter::DoCrouch);
		
		EnhancedInputComponent->BindAction(CrouchAction,ETriggerEvent::Completed,this,&APlayerCharacter::DoUnCrouch);
		
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Canceled, this,&APlayerCharacter::DoUnCrouch);
		
		
		if (AutoBhopFunction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this,
			                                   &APlayerCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
			                                   &APlayerCharacter::DoJumpEnd);
		}
		if (!AutoBhopFunction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
			                                   &APlayerCharacter::DoJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
			                                   &APlayerCharacter::DoJumpEnd);
		}
	}
}

FRotator APlayerCharacter::GetViewRotation() const
{
	FRotator ViewRotation = Super::GetViewRotation();
	ViewRotation.Roll = CurrentCameraRoll;
	return ViewRotation;
}

float APlayerCharacter::GetCameraInputDirection() const
{
	return CurrentCameraRoll > 90.0f ? -1.0f : 1.0f;
}

void APlayerCharacter::SetCameraAngle()
{
	if (!FirstPersonCamera)
	{
		return;
	}

	SetCameraFlipInputLocked(true);
	if (AController* InputController = GetController())
	{
		InputController->SetControlRotation(FRotator(0.0f, InputController->GetControlRotation().Yaw, 0.0f));
		if (APlayerController* PlayerController = Cast<APlayerController>(InputController))
		{
			PlayerController->RotationInput = FRotator::ZeroRotator;
		}
	}

	ClearSourceMovementInput();
	ConsumeMovementInputVector();
	StopJumping();
	GetCharacterMovement()->StopMovementImmediately();

	StartCameraRoll = CurrentCameraRoll;
	TargetCameraRoll = GetCharacterMovement()->GetGravityDirection().Z > 0.0f ? 180.0f : 0.0f;
	CameraFlipElapsed = 0.0f;
	UpdateCameraAngle(0.0f);
}

void APlayerCharacter::SetCameraFlipInputLocked(bool bLocked)
{
	if (bIsCameraFlipping == bLocked)
	{
		return;
	}

	bIsCameraFlipping = bLocked;
	if (bLocked)
	{
		CameraFlipInputController = GetController();
	}

	if (AController* InputController = CameraFlipInputController.Get())
	{
		InputController->SetIgnoreLookInput(bLocked);
		InputController->SetIgnoreMoveInput(bLocked);
	}

	if (!bLocked)
	{
		CameraFlipInputController.Reset();
	}
}

void APlayerCharacter::UpdateCameraAngle(float DeltaTime)
{
	if (!bIsCameraFlipping)
	{
		return;
	}

	if (!FirstPersonCamera || CurrentCameraRoll == TargetCameraRoll)
	{
		SetCameraFlipInputLocked(false);
		return;
	}

	CameraFlipElapsed += DeltaTime;
	const float Alpha = CameraFlipDuration > 0.0f
		? FMath::Clamp(CameraFlipElapsed / CameraFlipDuration, 0.0f, 1.0f)
		: 1.0f;
	const float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
	CurrentCameraRoll = Alpha >= 1.0f
		? TargetCameraRoll
		: FMath::Lerp(StartCameraRoll, TargetCameraRoll, SmoothAlpha);
	if (Alpha >= 1.0f)
	{
		SetCameraFlipInputLocked(false);
	}
}

void APlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	if (bIsCameraFlipping || IsMoveInputIgnored())
	{
		ClearSourceMovementInput();
		return;
	}

	if (bDevMode)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, "Input Detctive");
	}

	FVector2D Vector2D = Value.Get<FVector2D>();

	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		SourceMove->SetMovementInput(Vector2D.Y, Vector2D.X * GetCameraInputDirection());
	}
}

void APlayerCharacter::MoveInputReleased()
{
	ClearSourceMovementInput();
}

void APlayerCharacter::ClearSourceMovementInput()
{
	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		SourceMove->SetMovementInput(0.0f, 0.0f);
	}
}

void APlayerCharacter::MouseLookInput(const FInputActionValue& Value)
{
	if (bDevMode)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, "Mouse Spining Detective");
	}
	FVector2D V = Value.Get<FVector2D>();

	DoLook(V.X, V.Y);
}

void APlayerCharacter::Domove(float right, float left)
{
	if (!bIsCameraFlipping && GetController())
	{
		AddMovementInput(GetActorRightVector(), right * GetCameraInputDirection());
		AddMovementInput(GetActorForwardVector(), left);
	}
}

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (!bIsCameraFlipping && GetController())
	{
		const float InputDirection = GetCameraInputDirection();
		AddControllerYawInput(Yaw * InputDirection);
		AddControllerPitchInput(Pitch * InputDirection);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
void APlayerCharacter::DoJumpStart()
{
	if (bIsCameraFlipping || IsMoveInputIgnored())
	{
		return;
	}

	Jump();
}

void APlayerCharacter::DoJumpEnd()
{
	StopJumping();
}

void APlayerCharacter::DoCrouch()
{
	if (bIsCameraFlipping || IsMoveInputIgnored())
	{
		return;
	}

	Crouch();
}

void APlayerCharacter::DoUnCrouch()
{
	UnCrouch();
}

void APlayerCharacter::ShowVelocity()
{
	FVector Velocity = GetVelocity();

	if (const USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(
		GetCharacterMovement()))
	{
		FVector WishDir = FVector::ZeroVector;
		float WishSpeed = 0.0f;
		SourceMove->CalcWishDirAndSpeed(WishDir, WishSpeed);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(
			                                TEXT("mv: forward=%.3f side=%.3f"), SourceMove->GetForwardMove(),
			                                SourceMove->GetSideMove()));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(
			                                TEXT("wishdir: X=%.3f Y=%.3f | wishspeed=%.1f"), WishDir.X, WishDir.Y,
			                                WishSpeed));
	}

	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green,
	                                 FString::Printf(TEXT("MaxWalkSpeed: %f"), GetCharacterMovement()->MaxWalkSpeed));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(
		                                TEXT("Speed: %f %f %f Size2D:%f"), Velocity.X, Velocity.Y, Velocity.Z,
		                                Velocity.Size2D()));
}

/*
void APlayerCharacter::SwitchToFirstCam()
{
	if (FirstPersonCamera && DebugCamera)
	{
		FirstPersonCamera->Activate();
		DebugCamera->Deactivate();
	}
}
*/



void APlayerCharacter::Respawn()
{
	AActor* TargetSP = CurrentSpawnPoint;
	if (!IsValid(TargetSP))
	{
		TargetSP = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());
	}
	if (!IsValid(TargetSP))
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("[Player] Respawn() -> %s @ %s"),
	       *GetNameSafe(TargetSP), *TargetSP->GetActorLocation().ToString());

	SetCameraFlipInputLocked(false);
	ClearSourceMovementInput();
	ConsumeMovementInputVector();
	StopJumping();
	CurrentCameraRoll = 0.0f;
	StartCameraRoll = 0.0f;
	TargetCameraRoll = 0.0f;
	CameraFlipElapsed = 0.0f;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetGravityDirection(FVector::DownVector);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	if (AController* InputController = GetController())
	{
		InputController->SetControlRotation(FRotator(0.0f, TargetSP->GetActorRotation().Yaw, 0.0f));
		if (APlayerController* PlayerController = Cast<APlayerController>(InputController))
		{
			PlayerController->RotationInput = FRotator::ZeroRotator;
		}
	}
	SetActorLocationAndRotation(TargetSP->GetActorLocation(), TargetSP->GetActorRotation(), false, nullptr,
	                            ETeleportType::TeleportPhysics);
}

void APlayerCharacter::SetSpawnPoint(AActor* InPoint)
{
	CurrentSpawnPoint = InPoint;
	if (bDevMode && InPoint)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Yellow,
		                                 FString::Printf(TEXT("[Player] RespawnPoint Updated: %s @ %s"),
		                                                 *InPoint->GetName(),
		                                                 *InPoint->GetActorLocation().ToString()));
	}
}

