#pragma once

#include "CoreMinimal.h"
#include "WorldsBasicActor.h"
#include "MaterialTriggerBlock.generated.h"

UCLASS(Blueprintable)
class MEGAJAM_2026_API AMaterialTriggerBlock : public AWorldsBasicActor
{
	GENERATED_BODY()

protected:
	virtual bool ShouldFlipPlayerOnOverlap() const override;
};
