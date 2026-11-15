#include "Actors/PressurePlateLaunch.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

APressurePlateLaunch::APressurePlateLaunch()
{
	PlateColorIdle = FLinearColor(0.025f, 1.0f, 0.008f);
	PlateColorPressed = PlateColorIdle * 0.65f;
	PlateColorPressed.A = 1.0f;

	PlateSize = 90.0f;
	PressDepth = 6.0f;
	PressDuration = 0.1f;
	// 可反复踩：玩家被弹飞后离开触发盒，板子自动回弹，落回来再踩就再弹一次。
	// 每次"进入"只触发一次，站在板上不会持续施加推力。
	bOneShot = false;
}

void APressurePlateLaunch::ApplyPlateEffect(AActor* PressingActor)
{
	APawn* Pawn = Cast<APawn>(PressingActor);
	if (!Pawn)
	{
		return;
	}

	UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());
	if (!Movement)
	{
		return;
	}

	// 用 -重力方向当"上"，这样重力翻转后的玩家也能被正确弹向自己脚下的地面
	const FVector Up = -Movement->GetGravityDirection().GetSafeNormal();

	const FVector CurrentVelocity = Movement->Velocity;
	const FVector AlongUp = Up * FVector::DotProduct(CurrentVelocity, Up);
	const FVector Lateral = CurrentVelocity - AlongUp;

	Movement->Velocity = Lateral * FMath::Clamp(LateralVelocityRetain, 0.0f, 1.0f)
		+ Up * FMath::Max(LaunchSpeed, 0.0f);

	if (bForceFalling)
	{
		Movement->SetMovementMode(MOVE_Falling);
	}

	OnLaunchFired.Broadcast(Pawn, Movement->Velocity);
}
