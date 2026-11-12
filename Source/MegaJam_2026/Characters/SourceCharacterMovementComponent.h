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

	void SetMovementInput(float ForwardIn, float SideIn);

	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	FVector CalcWishVel() const;

	void CalcWishDirAndSpeed(FVector& OutWishDir, float& OutWishSpeed) const;

	void GetViewBasisVectors(FVector& OutForward, FVector& OutRight) const;

	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	float GetForwardMove() const { return mv_forwardMove; }

	UFUNCTION(BlueprintPure, Category = "Source Movement|WishVel")
	float GetSideMove() const { return mv_sideMove; }

protected:
	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_AirAcceleration = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_StopSpeed = 100.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Move Settings")
	float MaxGroundSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Move Settings")
	float Sv_Friction = 5.f;

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
