#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SourceCharacterMovementComponent.generated.h"

UCLASS()
class MEGAJAM_2026_API USourceCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USourceCharacterMovementComponent();

	UPROPERTY(EditAnywhere, Category = "Tools|AutoBhoping")
	bool EnableBunnyhoping = false;

	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;
	virtual float GetMaxSpeed() const override;
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Move Settings",
		meta = (ClampMin = "0.0", Units = "cm/s"))
	float MaxCrouchSpeed = 300.0f;

	void SetMovementInput(float ForwardIn, float SideIn);

	UFUNCTION(BlueprintPure, Category = "Movement|Mantle")
	bool IsMantling() const;

	UFUNCTION(BlueprintCallable, Category = "Movement|Mantle")
	bool TryStartMantle();

	UFUNCTION(BlueprintCallable, Category = "Movement|Mantle")
	void CancelMantle();

	virtual void SetGravityDirection(const FVector& GravityDir) override;
	virtual void OnTeleported() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Mantle",
		meta = (ClampMin = "0.0", Units = "cm",
			ToolTip = "Maximum ledge height above the character capsule center, measured opposite gravity."))
	float MantleMaxHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Mantle",
		meta = (ClampMin = "1.0", Units = "cm"))
	float MantleWallDistance = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Mantle",
		meta = (ClampMin = "0.1", Units = "s"))
	float MantleDuration = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Mantle")
	bool bDrawMantleDebug = false;

	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	FVector CalcWishVel() const;

	void CalcWishDirAndSpeed(FVector& OutWishDir, float& OutWishSpeed) const;

	void GetViewBasisVectors(FVector& OutForward, FVector& OutRight) const;

	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	float GetForwardMove() const { return mv_forwardMove; }

	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	float GetSideMove() const { return mv_sideMove; }

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	bool FindMantleTarget(FVector& OutTarget, FVector& OutLift, FVector& OutAcross,
		UPrimitiveComponent*& OutSurface) const;
	bool IsMantlePathClear(const FVector& Start, const FVector& End) const;
	void FinishMantle();

	static constexpr uint8 MantleMovementMode = 1;
	FVector MantleStart = FVector::ZeroVector;
	FVector MantleLift = FVector::ZeroVector;
	FVector MantleAcross = FVector::ZeroVector;
	FVector MantleTarget = FVector::ZeroVector;
	FVector MantleGravity = FVector::DownVector;
	TWeakObjectPtr<UPrimitiveComponent> MantleSurface;
	FTransform MantleSurfaceTransform;
	float MantleElapsed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_AirAcceleration = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_StopSpeed = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Move Settings")
	float MaxGroundSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_Friction =12.f;

	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_Accelerate = 12.f;

	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_AirCappingSpeed = 30.f;

	float mv_forwardMove = 0.f;
	float mv_sideMove = 0.f;
	float DebugLineLength = 500.f;

	virtual float GetCapppingAirAccleration() { return 100.0f; }

	void AirMove(float DeltaTime);

	void AirAcceleration(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);

	void WalkMove(float DeltaTime);

	void ApplyFriction(float DeltaTime);

	void Accelerate(FVector wishdir, float wishSpeed, float acceleration, float DeltaTime);

	FVector VectorScale(const FVector& InVector, double scale);

	float GetGroundFriction(float DeltaTime);

	void CalcVector(const FRotator& Angles, float Forward);

public:
	virtual void DrawCameraDebugline(bool bDevMode);
};
