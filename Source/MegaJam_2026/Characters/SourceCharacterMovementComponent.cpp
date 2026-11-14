#include "MegaJam_2026/Characters/SourceCharacterMovementComponent.h"

#include "MegaJam_2026/Characters/PlayerCharacter.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Math/RotationMatrix.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"

USourceCharacterMovementComponent::USourceCharacterMovementComponent()
{
	GroundFriction = 8.f;
	MaxAcceleration = 600.f;
	JumpZVelocity = 600.f;
}

float USourceCharacterMovementComponent::GetMaxSpeed() const
{
	return IsMovingOnGround() && IsCrouching() ? FMath::Max(0.0f, MaxCrouchSpeed) : Super::GetMaxSpeed();
}

void USourceCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	if (!HasValidData())
	{
		Super::Crouch(bClientSimulation);
		return;
	}

	const FVector PreviousLocation = UpdatedComponent->GetComponentLocation();
	const bool bWasCrouching = IsCrouching();
	Super::Crouch(bClientSimulation);
	if (HasValidData() && !bWasCrouching && IsCrouching())
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(CharacterOwner))
		{
			Player->BeginCrouchCameraTransition(PreviousLocation - UpdatedComponent->GetComponentLocation());
		}
	}
}

void USourceCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	if (!HasValidData())
	{
		Super::UnCrouch(bClientSimulation);
		return;
	}

	const FVector PreviousLocation = UpdatedComponent->GetComponentLocation();
	const bool bWasCrouching = IsCrouching();
	Super::UnCrouch(bClientSimulation);
	if (HasValidData() && bWasCrouching && !IsCrouching())
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(CharacterOwner))
		{
			Player->BeginCrouchCameraTransition(PreviousLocation - UpdatedComponent->GetComponentLocation());
		}
	}
}

bool USourceCharacterMovementComponent::IsMantling() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == MantleMovementMode;
}

bool USourceCharacterMovementComponent::IsMantlePathClear(const FVector& Start, const FVector& End) const
{
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MantlePath), false, CharacterOwner);
	FCollisionResponseParams ResponseParams;
	Capsule->InitSweepCollisionParams(QueryParams, ResponseParams);
	FHitResult Hit;
	return !GetWorld()->SweepSingleByChannel(Hit, Start, End, Capsule->GetComponentQuat(),
		Capsule->GetCollisionObjectType(), Capsule->GetCollisionShape(), QueryParams, ResponseParams);
}

bool USourceCharacterMovementComponent::FindMantleTarget(FVector& OutTarget, FVector& OutLift,
	FVector& OutAcross, UPrimitiveComponent*& OutSurface) const
{
	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const FVector Up = -GetGravityDirection();
	const FVector Forward = FVector::VectorPlaneProject(
		CharacterOwner->GetControlRotation().Vector(), Up).GetSafeNormal();
	if (Forward.IsNearlyZero() || MantleMaxHeight <= 0.0f || MantleWallDistance <= 0.0f)
	{
		return false;
	}

	const FVector Start = UpdatedComponent->GetComponentLocation();
	const float Radius = Capsule->GetScaledCapsuleRadius();
	const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const float ProbeRadius = FMath::Min(12.0f, Radius * 0.5f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MantleTarget), false, CharacterOwner);
	FCollisionResponseParams ResponseParams;
	Capsule->InitSweepCollisionParams(QueryParams, ResponseParams);
	const ECollisionChannel Channel = Capsule->GetCollisionObjectType();
	const float ProbeOffsets[] = {0.0f, -FMath::Min(80.0f, HalfHeight * 0.65f)};

	for (const float ProbeOffset : ProbeOffsets)
	{
		const FVector WallStart = Start + Up * ProbeOffset;
		const FVector WallEnd = WallStart + Forward * (Radius + MantleWallDistance);
		FHitResult WallHit;
		const bool bHitWall = GetWorld()->SweepSingleByChannel(WallHit, WallStart, WallEnd,
			FQuat::Identity, Channel, FCollisionShape::MakeSphere(ProbeRadius), QueryParams, ResponseParams);
		if (bDrawMantleDebug)
		{
			DrawDebugLine(GetWorld(), WallStart, WallEnd, bHitWall ? FColor::Yellow : FColor::Red,
				false, 0.0f, 0, 2.0f);
		}
		if (!bHitWall || !WallHit.IsValidBlockingHit()
			|| FMath::Abs(FVector::DotProduct(WallHit.ImpactNormal, Up)) > 0.25f
			|| FVector::DotProduct(Forward, -WallHit.ImpactNormal) < 0.7f)
		{
			if (bDrawMantleDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Mantle wall rejected: hit=%d normal=%s"), bHitWall,
					*WallHit.ImpactNormal.ToCompactString());
			}
			continue;
		}

		UPrimitiveComponent* Surface = WallHit.GetComponent();
		if (!Surface)
		{
			continue;
		}

		const FVector Inward = FVector::VectorPlaneProject(-WallHit.ImpactNormal, Up).GetSafeNormal();
		const FVector EdgeStart = WallHit.ImpactPoint + Inward * 3.0f
			+ Up * (MantleMaxHeight + 10.0f - FVector::DotProduct(WallHit.ImpactPoint - Start, Up));
		const FVector EdgeEnd = EdgeStart - Up * (MantleMaxHeight + 70.0f);
		FHitResult EdgeHit;
		if (!GetWorld()->LineTraceSingleByChannel(EdgeHit, EdgeStart, EdgeEnd, Channel, QueryParams,
			ResponseParams) || EdgeHit.GetComponent() != Surface || !IsWalkable(EdgeHit))
		{
			if (bDrawMantleDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Mantle edge rejected: hit=%d same=%d walkable=%d point=%s"),
					EdgeHit.IsValidBlockingHit(), EdgeHit.GetComponent() == Surface, IsWalkable(EdgeHit),
					*EdgeHit.ImpactPoint.ToCompactString());
			}
			continue;
		}

		const float EdgeHeight = FVector::DotProduct(EdgeHit.ImpactPoint - Start, Up);
		if (EdgeHeight > MantleMaxHeight || EdgeHeight < -60.0f)
		{
			continue;
		}

		const FVector TopStart = EdgeHit.ImpactPoint + Inward * (Radius + 12.0f) + Up * 15.0f;
		FHitResult TopHit;
		if (!GetWorld()->LineTraceSingleByChannel(TopHit, TopStart, TopStart - Up * 30.0f,
			Channel, QueryParams, ResponseParams) || TopHit.GetComponent() != Surface || !IsWalkable(TopHit))
		{
			if (bDrawMantleDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Mantle top rejected: hit=%d same=%d walkable=%d point=%s"),
					TopHit.IsValidBlockingHit(), TopHit.GetComponent() == Surface, IsWalkable(TopHit),
					*TopHit.ImpactPoint.ToCompactString());
			}
			continue;
		}

		bool bSupported = true;
		const FVector Side = FVector::CrossProduct(Up, Inward).GetSafeNormal();
		const FVector SupportOffsets[] = {Side * Radius, -Side * Radius, Inward * Radius, -Inward * Radius};
		for (const FVector& SupportOffset : SupportOffsets)
		{
			FHitResult SupportHit;
			const FVector SupportStart = TopHit.ImpactPoint + SupportOffset + Up * 15.0f;
			if (!GetWorld()->LineTraceSingleByChannel(SupportHit, SupportStart, SupportStart - Up * 30.0f,
				Channel, QueryParams, ResponseParams) || SupportHit.GetComponent() != Surface || !IsWalkable(SupportHit))
			{
				bSupported = false;
				break;
			}
		}
		if (!bSupported)
		{
			if (bDrawMantleDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Mantle support rejected"));
			}
			continue;
		}

		const FVector Target = TopHit.ImpactPoint + Up * (HalfHeight + 2.15f);
		const FVector Lift = Start + Up * (FVector::DotProduct(Target - Start, Up) + 5.0f);
		const FVector Across = Target + Up * 5.0f;
		if (GetWorld()->OverlapBlockingTestByChannel(Target, Capsule->GetComponentQuat(), Channel,
			Capsule->GetCollisionShape(), QueryParams, ResponseParams)
			|| !IsMantlePathClear(Start, Lift) || !IsMantlePathClear(Lift, Across)
			|| !IsMantlePathClear(Across, Target))
		{
			if (bDrawMantleDebug)
			{
				UE_LOG(LogTemp, Warning, TEXT("Mantle path rejected: target=%s"), *Target.ToCompactString());
			}
			continue;
		}

		OutTarget = Target;
		OutLift = Lift;
		OutAcross = Across;
		OutSurface = Surface;
		if (bDrawMantleDebug)
		{
			DrawDebugLine(GetWorld(), EdgeStart, EdgeEnd, FColor::Cyan, false, 0.5f, 0, 2.0f);
			DrawDebugLine(GetWorld(), Start, Lift, FColor::Green, false, 0.5f, 0, 2.0f);
			DrawDebugLine(GetWorld(), Lift, Across, FColor::Green, false, 0.5f, 0, 2.0f);
			DrawDebugCapsule(GetWorld(), Target, HalfHeight, Radius, Capsule->GetComponentQuat(),
				FColor::Green, false, 0.5f);
		}
		return true;
	}
	return false;
}

bool USourceCharacterMovementComponent::TryStartMantle()
{
	if (bDrawMantleDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mantle attempt: mode=%d custom=%d falling=%d ground=%d role=%d"),
			MovementMode.GetValue(), CustomMovementMode, IsFalling(), IsMovingOnGround(),
			CharacterOwner ? static_cast<int32>(CharacterOwner->GetLocalRole()) : -1);
	}
	if (!HasValidData() || IsMantling() || (!IsMovingOnGround() && !IsFalling())
		|| IsCrouching() || bWantsToCrouch || HasAnimRootMotion() || !UpdatedComponent->IsQueryCollisionEnabled()
		|| CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		if (bDrawMantleDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mantle precondition rejected"));
		}
		return false;
	}

	UPrimitiveComponent* Surface = nullptr;
	if (!FindMantleTarget(MantleTarget, MantleLift, MantleAcross, Surface))
	{
		if (bDrawMantleDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mantle target rejected"));
		}
		return false;
	}

	MantleStart = UpdatedComponent->GetComponentLocation();
	MantleElapsed = 0.0f;
	MantleGravity = GetGravityDirection();
	MantleSurface = Surface;
	MantleSurfaceTransform = Surface->GetComponentTransform();
	CharacterOwner->StopJumping();
	CharacterOwner->ConsumeMovementInputVector();
	SetMovementInput(0.0f, 0.0f);
	StopMovementImmediately();
	ClearAccumulatedForces();
	SetMovementMode(MOVE_Custom, MantleMovementMode);
	return IsMantling();
}

void USourceCharacterMovementComponent::CancelMantle()
{
	if (IsMantling())
	{
		StopMovementImmediately();
		SetMovementMode(MOVE_Falling);
	}
}

void USourceCharacterMovementComponent::FinishMantle()
{
	StopMovementImmediately();
	FFindFloorResult Floor;
	FindFloor(UpdatedComponent->GetComponentLocation(), Floor, false);
	SetMovementMode(Floor.IsWalkableFloor() ? MOVE_Walking : MOVE_Falling);
}

void USourceCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (!IsMantling())
	{
		Super::PhysCustom(DeltaTime, Iterations);
		return;
	}
	if (!HasValidData() || DeltaTime < MIN_TICK_TIME)
	{
		return;
	}
	if (!MantleSurface.IsValid() || !MantleSurface->IsQueryCollisionEnabled()
		|| !GetGravityDirection().Equals(MantleGravity)
		|| !MantleSurface->GetComponentTransform().Equals(MantleSurfaceTransform)
		|| HasAnimRootMotion())
	{
		CancelMantle();
		return;
	}

	const float Duration = FMath::Max(MantleDuration, 0.1f);
	const float EndTime = FMath::Min(MantleElapsed + DeltaTime, Duration);
	while (MantleElapsed < EndTime && IsMantling())
	{
		const bool bLifting = MantleElapsed < Duration * 0.55f;
		const bool bCrossing = !bLifting && MantleElapsed < Duration * 0.9f;
		const float PhaseStart = bLifting ? 0.0f : (bCrossing ? Duration * 0.55f : Duration * 0.9f);
		const float PhaseEnd = bLifting ? Duration * 0.55f : (bCrossing ? Duration * 0.9f : Duration);
		const FVector PhaseFrom = bLifting ? MantleStart : (bCrossing ? MantleLift : MantleAcross);
		const FVector PhaseTo = bLifting ? MantleLift : (bCrossing ? MantleAcross : MantleTarget);
		MantleElapsed = FMath::Min(EndTime, PhaseEnd);
		const float Alpha = FMath::Clamp((MantleElapsed - PhaseStart) / (PhaseEnd - PhaseStart), 0.0f, 1.0f);
		const float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
		const FVector Target = FMath::Lerp(PhaseFrom, PhaseTo, SmoothAlpha);
		FHitResult Hit;
		SafeMoveUpdatedComponent(Target - UpdatedComponent->GetComponentLocation(),
			UpdatedComponent->GetComponentQuat(), true, Hit);
		if (!HasValidData() || !IsMantling())
		{
			return;
		}
		if (Hit.bBlockingHit || !GetGravityDirection().Equals(MantleGravity))
		{
			CancelMantle();
			return;
		}
	}
	Velocity = FVector::ZeroVector;
	if (IsMantling() && MantleElapsed >= Duration)
	{
		FinishMantle();
	}
}

void USourceCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,
	uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == MantleMovementMode && !IsMantling())
	{
		MantleSurface.Reset();
		MantleElapsed = 0.0f;
		SetMovementInput(0.0f, 0.0f);
	}
}

void USourceCharacterMovementComponent::SetGravityDirection(const FVector& GravityDir)
{
	if (!GravityDir.IsNearlyZero() && !GravityDir.GetSafeNormal().Equals(GetGravityDirection()))
	{
		CancelMantle();
	}
	Super::SetGravityDirection(GravityDir);
}

void USourceCharacterMovementComponent::OnTeleported()
{
	CancelMantle();
	Super::OnTeleported();
}

void USourceCharacterMovementComponent::SetMovementInput(float ForwardIn, float SideIn)
{
	mv_forwardMove = FMath::Clamp(ForwardIn, -1.0f, 1.0f);
	mv_sideMove = FMath::Clamp(SideIn, -1.0f, 1.0f);
}

void USourceCharacterMovementComponent::GetViewBasisVectors(FVector& OutForward, FVector& OutRight) const
{
	const float ViewYaw = CharacterOwner ? CharacterOwner->GetControlRotation().Yaw : 0.0f;
	const FRotator YawRotation(0.0f, ViewYaw, 0.0f);
	const FMatrix ViewBasis = FRotationMatrix(YawRotation);

	OutForward = ViewBasis.GetUnitAxis(EAxis::X);
	OutRight = ViewBasis.GetUnitAxis(EAxis::Y);
}

FVector USourceCharacterMovementComponent::CalcWishVel() const
{
	float F = mv_forwardMove;
	float S = mv_sideMove;
	const float InputSpdSq = F * F + S * S;
	if (InputSpdSq > 1.0f)
	{
		const float Ratio = 1.0f / FMath::Sqrt(InputSpdSq);
		F *= Ratio;
		S *= Ratio;
	}

	FVector ViewForward, ViewRight;
	GetViewBasisVectors(ViewForward, ViewRight);
	FVector WishVel = ViewForward * F + ViewRight * S;

	WishVel.Z = 0.0f;

	return WishVel;
}

void USourceCharacterMovementComponent::CalcWishDirAndSpeed(FVector& OutWishDir, float& OutWishSpeed) const
{
	const FVector WishVel = CalcWishVel();
	const float WishSpeedScalar = WishVel.Size();

	OutWishDir = WishVel.GetSafeNormal();

	const float SpeedLimit = IsMovingOnGround() && IsCrouching() ? GetMaxSpeed() : MaxGroundSpeed;
	OutWishSpeed = FMath::Min(WishSpeedScalar * SpeedLimit, SpeedLimit);
}

void USourceCharacterMovementComponent::AirMove(float DeltaTime)
{
	FVector wishdir = FVector::ZeroVector;
	float wishSpeed = 0.0f;
	CalcWishDirAndSpeed(wishdir, wishSpeed);

	AirAcceleration(wishdir, wishSpeed, Sv_AirAcceleration, DeltaTime);
}

void USourceCharacterMovementComponent::AirAcceleration(FVector wishdir, float wishSpeed, float acceleration,
                                                        float DeltaTime)
{
	if (!IsFalling())
	{
		return;
	}

	if (wishSpeed <= 0.0f || wishdir.IsNearlyZero())
	{
		return;
	}

	const float WishSpd = FMath::Min(wishSpeed, Sv_AirCappingSpeed);

	float addSpeed = 0.0f;
	const float crtspeed = Velocity.Dot(wishdir);
	float accelspeed = 0.0f;

	addSpeed = WishSpd - crtspeed;

	if (addSpeed <= 0)
	{
		return;
	}

	accelspeed = wishSpeed * DeltaTime * acceleration;

	if (accelspeed > addSpeed)
	{
		accelspeed = addSpeed;
	}

	Velocity += accelspeed * wishdir;
}

void USourceCharacterMovementComponent::WalkMove(float DeltaTime)
{
	FVector wishdir = FVector::ZeroVector;
	float wishSpeed = 0.0f;
	CalcWishDirAndSpeed(wishdir, wishSpeed);

	if (Velocity.Z != 0)
	{
		Velocity.Z = 0;
	}

	Accelerate(wishdir, wishSpeed, Sv_Accelerate, DeltaTime);
	if (IsCrouching())
	{
		Velocity = Velocity.GetClampedToMaxSize(GetMaxSpeed());
	}
}

void USourceCharacterMovementComponent::Accelerate(FVector wishdir, float wishSpeed, float acceleration,
                                                  float DeltaTime)
{
	float addspeed;
	float accelspeed;
	float crtspeed;

	crtspeed = Velocity.Dot(wishdir);
	addspeed = wishSpeed - crtspeed;
	if (addspeed <= 0)
	{
		return;
	}

	accelspeed = wishSpeed * DeltaTime * acceleration;

	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}

	Velocity += accelspeed * wishdir;
}

void USourceCharacterMovementComponent::ApplyFriction(float DeltaTime)
{
	float Speeding = Velocity.Size2D();
	float Control, NewSpeed, drop, Friction;
	if (Speeding < 0.1f)
	{
		Velocity.X = 0;
		Velocity.Y = 0;
		return;
	}

	drop = 0;
	Control = (Speeding < Sv_StopSpeed) ? Sv_StopSpeed : Speeding;
	Friction = FMath::Max(Sv_Friction, 0.0f);
	drop += Control * Friction * DeltaTime;
	NewSpeed = Speeding - drop;

	if (NewSpeed < 0.f)
	{
		NewSpeed = 0;
	}

	NewSpeed /= Speeding;

	Velocity = VectorScale(Velocity, NewSpeed);
}

void USourceCharacterMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid,
                                                     float BrakingDeceleration)
{
	if (const APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner))
	{
		if (PlayerChar->bDevMode)
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0, FColor::Red, TEXT("Custom CalcVelocity Avaliable"));
			}

			FVector WishDir = FVector::ZeroVector;
			float WishSpeed = 0.0f;
			CalcWishDirAndSpeed(WishDir, WishSpeed);

			DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CalcWishVel() * DebugLineLength,
			              FColor::Yellow, false, -1.0f, 0, 3.0f);
			DrawDebugLine(GetWorld(), GetActorLocation(),
			              GetActorLocation() + Velocity.GetSafeNormal2D() * 300.0f, FColor::Red, false, -1.0f, 0, 3.0f);

			GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(
				                                TEXT("mv: f=%.3f s=%.3f | wishdir: X=%.3f Y=%.3f | wishspeed=%.1f | vel=%.1f"),
				                                mv_forwardMove, mv_sideMove, WishDir.X, WishDir.Y, WishSpeed,
				                                Velocity.Size2D()));
		}
	}

	if (APlayerCharacter* RenderCamPointer = Cast<APlayerCharacter>(CharacterOwner))
	{
		if (RenderCamPointer)
		{
			DrawCameraDebugline(RenderCamPointer->bDevMode);
		}
		else
		{
			return;
		}
	}

	if (!HasValidData() || HasAnimRootMotion() || DeltaTime < MIN_TICK_TIME
		|| (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy && !bWasSimulatingRootMotion))
	{
		return;
	}

	if (IsFalling())
	{
		AirMove(DeltaTime);
		return;
	}

	if (IsMovingOnGround())
	{
		ApplyFriction(DeltaTime);
		WalkMove(DeltaTime);
		return;
	}
}

FVector USourceCharacterMovementComponent::VectorScale(const FVector& InVector, double scale)
{
	return InVector * scale;
}

float USourceCharacterMovementComponent::GetGroundFriction(float DeltaTime)
{
	return GroundFriction;
}

void USourceCharacterMovementComponent::CalcVector(const FRotator& Angles, float Forward)
{
}

void USourceCharacterMovementComponent::DrawCameraDebugline(bool bDevMode)
{
	APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(CharacterOwner);

	if (!PlayerChar || !bDevMode)
	{
		return;
	}

	FVector ViewForward, ViewRight;
	GetViewBasisVectors(ViewForward, ViewRight);
	const FVector Origin = GetActorLocation();

	DrawDebugLine(GetWorld(), Origin, Origin + ViewForward * DebugLineLength, FColor::Green, false, -1.0f, 0, 3.0f);
	DrawDebugLine(GetWorld(), Origin, Origin + ViewRight * DebugLineLength, FColor::Blue, false, -1.0f, 0, 3.0f);
	DrawDebugLine(GetWorld(), Origin, Origin + CalcWishVel() * DebugLineLength, FColor::Yellow, false, -1.0f, 0, 3.0f);
}
