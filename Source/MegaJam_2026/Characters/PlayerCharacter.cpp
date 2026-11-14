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
#include "MegaJam_2026/Rendering/FogViewExtension.h"
#include "DynamicRHI.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"
#include "RenderTimer.h"
#include "RHIStats.h"

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
	FirstPersonSpringArm->AddTickPrerequisiteActor(this);
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
	if (GetNetMode() != NM_DedicatedServer)
	{
		RegisterFogViewExtension();
	}
	if (FirstPersonCamera)
	{
		FirstPersonCamera->PostProcessBlendWeight = 1.0f;
		FirstPersonCamera->PostProcessSettings.bOverride_MotionBlurAmount = true;
		FirstPersonCamera->PostProcessSettings.MotionBlurAmount = 0.15f;
		FirstPersonCamera->PostProcessSettings.bOverride_MotionBlurMax = true;
		FirstPersonCamera->PostProcessSettings.MotionBlurMax = 1.0f;
		FirstPersonCamera->PostProcessSettings.bOverride_MotionBlurTargetFPS = true;
		FirstPersonCamera->PostProcessSettings.MotionBlurTargetFPS = 60;
	}
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

void APlayerCharacter::BeginCrouchCameraTransition(const FVector& CapsuleLocationCompensation)
{
	if (!FirstPersonSpringArm || bIsCameraFlipping || CrouchCameraTransitionDuration <= 0.0f)
	{
		ResetCrouchCamera();
		return;
	}

	FirstPersonSpringArm->TargetOffset += CapsuleLocationCompensation;
	CrouchCameraStartOffset = FirstPersonSpringArm->TargetOffset;
	CrouchCameraElapsed = 0.0f;
}

void APlayerCharacter::UpdateCrouchCamera(float DeltaTime)
{
	if (!FirstPersonSpringArm || CrouchCameraStartOffset.IsNearlyZero())
	{
		return;
	}

	CrouchCameraElapsed += DeltaTime;
	const float Alpha = CrouchCameraTransitionDuration > 0.0f
		? FMath::Clamp(CrouchCameraElapsed / CrouchCameraTransitionDuration, 0.0f, 1.0f)
		: 1.0f;
	const float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
	FirstPersonSpringArm->TargetOffset = FMath::Lerp(CrouchCameraStartOffset, FVector::ZeroVector, SmoothAlpha);
	if (Alpha >= 1.0f)
	{
		ResetCrouchCamera();
	}
}

void APlayerCharacter::ResetCrouchCamera()
{
	CrouchCameraStartOffset = FVector::ZeroVector;
	CrouchCameraElapsed = 0.0f;
	if (FirstPersonSpringArm)
	{
		FirstPersonSpringArm->TargetOffset = FVector::ZeroVector;
	}
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CancelMantle();
	SetCameraFlipInputLocked(false);
	showFramepos(false);
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCameraAngle(DeltaTime);
	UpdateCrouchCamera(DeltaTime);
	UpdateJumpInput(DeltaTime);
	showFramepos(bShowFPS);
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
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
			&APlayerCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this,
			&APlayerCharacter::DoJumpHeld);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this,
			&APlayerCharacter::DoJumpEnd);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Canceled, this,
			&APlayerCharacter::DoJumpEnd);
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

void APlayerCharacter::showFramepos(bool bShouldShowFPS)
{
	bShowFPS = bShouldShowFPS;
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	const uint64 MessageKey = 0x4650530000000000ull | static_cast<uint64>(GetUniqueID());
	if (!GEngine || !bShowFPS || !IsLocallyControlled())
	{
		if (GEngine)
		{
			GEngine->RemoveOnScreenDebugMessage(MessageKey);
		}
		FrameStatsStartTime = 0.0;
		FrameStatsLastFrameTime = 0.0;
		FrameStatsPeakFrameTime = 0.0;
		FrameStatsFrameCount = 0;
		return;
	}

	const double CurrentTime = FPlatformTime::Seconds();
	if (FrameStatsStartTime == 0.0)
	{
		FrameStatsStartTime = CurrentTime;
		FrameStatsLastFrameTime = CurrentTime;
		return;
	}

	FrameStatsPeakFrameTime = FMath::Max(FrameStatsPeakFrameTime, CurrentTime - FrameStatsLastFrameTime);
	FrameStatsLastFrameTime = CurrentTime;
	++FrameStatsFrameCount;
	const double SampleDuration = CurrentTime - FrameStatsStartTime;
	if (SampleDuration < 0.25)
	{
		return;
	}

	const double FramesPerSecond = FrameStatsFrameCount / SampleDuration;
	const double FrameMilliseconds = SampleDuration * 1000.0 / FrameStatsFrameCount;
	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	constexpr double BytesPerMiB = 1024.0 * 1024.0;
	const double SystemUsedMemory = static_cast<double>(MemoryStats.TotalPhysical)
		- static_cast<double>(MemoryStats.AvailablePhysical);
	FString StatsText = FString::Printf(
		TEXT("FPS: %.1f | Frame avg: %.2f ms | Peak (sample): %.2f ms\n")
		TEXT("CPU Game: %.2f ms | Render: %.2f ms | RHI: %.2f ms (latest)\n")
		TEXT("Process RAM: %.0f MiB | Peak: %.0f MiB | Commit/virtual: %.0f MiB\n")
		TEXT("System RAM used/total: %.0f / %.0f MiB | Available: %.0f MiB\n"),
		FramesPerSecond, FrameMilliseconds, FrameStatsPeakFrameTime * 1000.0,
		FPlatformTime::ToMilliseconds(GGameThreadTime),
		FPlatformTime::ToMilliseconds(GRenderThreadTime),
		FPlatformTime::ToMilliseconds(GRHIThreadTime),
		MemoryStats.UsedPhysical / BytesPerMiB, MemoryStats.PeakUsedPhysical / BytesPerMiB,
		MemoryStats.UsedVirtual / BytesPerMiB, SystemUsedMemory / BytesPerMiB,
		MemoryStats.TotalPhysical / BytesPerMiB, MemoryStats.AvailablePhysical / BytesPerMiB);

	if (GDynamicRHI)
	{
		const uint32 GPUFrameCycles = RHIGetGPUFrameCycles();
		StatsText += GPUFrameCycles > 0
			? FString::Printf(TEXT("GPU: %.2f ms (latest)\n"), FPlatformTime::ToMilliseconds(GPUFrameCycles))
			: TEXT("GPU: N/A (timing unavailable)\n");

		FRHIMemoryStats VideoMemoryStats;
		RHIGetMemoryStats(VideoMemoryStats);
		if (VideoMemoryStats.BudgetLocal > 0)
		{
			StatsText += FString::Printf(
				TEXT("Process VRAM used/budget: %.0f / %.0f MiB%s\n"),
				VideoMemoryStats.UsedLocal / BytesPerMiB, VideoMemoryStats.BudgetLocal / BytesPerMiB,
				VideoMemoryStats.UsedLocal > VideoMemoryStats.BudgetLocal ? TEXT(" [OVER BUDGET]") : TEXT(""));
		}
		else
		{
			StatsText += TEXT("Process VRAM used/budget: N/A (RHI unsupported)\n");
		}
		if (VideoMemoryStats.BudgetSystem > 0)
		{
			StatsText += FString::Printf(TEXT("GPU system/shared used/budget: %.0f / %.0f MiB\n"),
				VideoMemoryStats.UsedSystem / BytesPerMiB, VideoMemoryStats.BudgetSystem / BytesPerMiB);
		}

		FTextureMemoryStats TextureMemoryStats;
		RHIGetTextureMemoryStats(TextureMemoryStats);
		if (TextureMemoryStats.DedicatedVideoMemory > 0)
		{
			StatsText += FString::Printf(TEXT("Dedicated VRAM capacity: %.0f MiB\n"),
				TextureMemoryStats.DedicatedVideoMemory / BytesPerMiB);
		}
		StatsText += FString::Printf(TEXT("Textures streaming/non-streaming: %.0f / %.0f MiB | Pool budget: %.0f MiB\n"),
			TextureMemoryStats.StreamingMemorySize / BytesPerMiB,
			TextureMemoryStats.NonStreamingMemorySize / BytesPerMiB,
			TextureMemoryStats.TexturePoolSize / BytesPerMiB);
	}
	else
	{
		StatsText += TEXT("GPU / VRAM: N/A (no RHI)\n");
	}

	StatsText += GIsEditor
		? TEXT("PIE: memory and renderer stats include the editor process; threads overlap.")
		: TEXT("Memory is process-wide; CPU/GPU stages overlap, do not add their times.");
	GEngine->AddOnScreenDebugMessage(MessageKey, 2.0f, FColor::Cyan, StatsText, false);
	FrameStatsStartTime = CurrentTime;
	FrameStatsPeakFrameTime = 0.0;
	FrameStatsFrameCount = 0;
#endif
}

void APlayerCharacter::SetCameraAngle()
{
	if (!FirstPersonCamera)
	{
		return;
	}

	CancelMantle();
	ResetJumpInputState();
	ResetCrouchCamera();
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
	if (bIsCameraFlipping || IsMantling() || IsMoveInputIgnored())
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
	if (!bIsCameraFlipping && !IsMantling() && !IsMoveInputIgnored() && GetController())
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
	if (bIsCameraFlipping || IsMantling() || IsMoveInputIgnored())
	{
		return;
	}

	bJumpInputHeld = true;
	bMantleConsumedForHold = false;
	JumpHeldTime = 0.0f;
	Jump();
}

void APlayerCharacter::DoJumpHeld()
{
	if (AutoBhopFunction && bJumpInputHeld && !bMantleConsumedForHold
		&& !bIsCameraFlipping && !IsMantling() && !IsMoveInputIgnored()
		&& GetCharacterMovement()->IsMovingOnGround())
	{
		Jump();
	}
}

void APlayerCharacter::DoJumpEnd()
{
	StopJumping();
	ResetJumpInputState();
}

void APlayerCharacter::DoCrouch()
{
	if (bIsCameraFlipping || IsMantling() || IsMoveInputIgnored())
	{
		return;
	}

	Crouch();
}

void APlayerCharacter::DoUnCrouch()
{
	if (!IsMantling())
	{
		UnCrouch();
	}
}

void APlayerCharacter::UpdateJumpInput(float DeltaTime)
{
	if (!bJumpInputHeld || bIsCameraFlipping || IsMoveInputIgnored())
	{
		return;
	}

	JumpHeldTime += DeltaTime;
	if (!bEnableMantling || bMantleConsumedForHold || JumpHeldTime < MantleHoldTime)
	{
		return;
	}

	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		if (SourceMove->TryStartMantle())
		{
			bMantleConsumedForHold = true;
		}
	}
}

void APlayerCharacter::ResetJumpInputState()
{
	bJumpInputHeld = false;
	bMantleConsumedForHold = false;
	JumpHeldTime = 0.0f;
}

void APlayerCharacter::CancelMantle()
{
	if (USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		SourceMove->CancelMantle();
	}
}

bool APlayerCharacter::IsMantling() const
{
	if (const USourceCharacterMovementComponent* SourceMove = Cast<USourceCharacterMovementComponent>(GetCharacterMovement()))
	{
		return SourceMove->IsMantling();
	}
	return false;
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
	CancelMantle();
	ResetJumpInputState();
	ResetCrouchCamera();
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

