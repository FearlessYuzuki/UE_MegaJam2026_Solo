#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundBlock.generated.h"

class UStaticMeshComponent;

UCLASS()
class MEGAJAM_2026_API AGroundBlock : public AActor
{
	GENERATED_BODY()

public:
	AGroundBlock();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GroundMesh;
};
