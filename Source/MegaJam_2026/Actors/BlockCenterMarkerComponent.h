#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "BlockCenterMarkerComponent.generated.h"

UCLASS()
class MEGAJAM_2026_API UBlockCenterMarkerComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UBlockCenterMarkerComponent();
	void RefreshForSurface(UStaticMeshComponent* Surface);
};
