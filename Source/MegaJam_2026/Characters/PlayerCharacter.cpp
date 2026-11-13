#include "MegaJam_2026/Characters/PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
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

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCamera"));
	FirstPersonSpringArm = CreateDefaultSubobject<USpringArmComponent>(FName("FirstPersonSpringArm"));

	FirstPersonSpringArm->SetupAttachment(RootComponent);
	FirstPersonCamera->SetupAttachment(FirstPersonSpringArm, USpringArmComponent::SocketName);

	FirstPersonSpringArm->TargetArmLength = 111.f;
	FirstPersonCamera->FieldOfView = 103.f;
	FirstPersonSpringArm->bUsePawnControlRotation = false;
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonSpringArm->SocketOffset = FVector(155.0f, 0.0f, 54.0f);

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
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

void APlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	if (bDevMode)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, "Input Detctive");
	}

	FVector2D Vector2D = Value.Get<FVector2D>();

	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		SourceMove->SetMovementInput(Vector2D.Y, Vector2D.X);
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
	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), right);
		AddMovementInput(GetActorForwardVector(), left);
	}
}

void APlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController())
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
void APlayerCharacter::DoJumpStart()
{
	Jump();
}

void APlayerCharacter::DoJumpEnd()
{
	StopJumping();
}

void APlayerCharacter::DoCrouch()
{
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

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
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

