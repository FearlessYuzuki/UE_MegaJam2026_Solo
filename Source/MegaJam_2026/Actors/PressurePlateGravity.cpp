#include "Actors/PressurePlateGravity.h"

#include "Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

APressurePlateGravity::APressurePlateGravity()
{
	PlateColorIdle = FLinearColor(0.65f, 0.008f, 1.0f);
	PlateColorPressed = PlateColorIdle * 0.65f;
	PlateColorPressed.A = 1.0f;

	PlateSize = 90.0f;
	PressDepth = 5.0f;
	PressDuration = 0.14f;
	bOneShot = true;
}

void APressurePlateGravity::ApplyPlateEffect(AActor* PressingActor)
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

	const FVector CurrentGravity = Movement->GetGravityDirection();

	FVector TargetGravity = FVector::DownVector;
	switch (FlipMode)
	{
	case EGravityPlateMode::ForceUp:
		TargetGravity = FVector::UpVector;
		break;
	case EGravityPlateMode::ForceDown:
		TargetGravity = FVector::DownVector;
		break;
	case EGravityPlateMode::Toggle:
	default:
		TargetGravity = -CurrentGravity;
		break;
	}

	if (TargetGravity.GetSafeNormal().Equals(CurrentGravity))
	{
		// 已经是目标方向，不重复翻转（否则镜头会白翻滚一次）
		return;
	}

	if (bStopMovementOnFlip)
	{
		Movement->StopMovementImmediately();
	}

	// 与 AWorldsBasicActor::OnTriggerBoxBeginOverlap 里完全一致的两步：
	// 换重力方向 -> 让角色把镜头/输入按新朝向重新摆正
	Movement->SetGravityDirection(TargetGravity);

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(Pawn))
	{
		Player->SetCameraAngle();
	}

	OnGravityFlipped.Broadcast(Pawn, Movement->GetGravityDirection());
}
