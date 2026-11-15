#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LoopSaveGame.generated.h"

USTRUCT()
struct FLoopSavedMaterial
{
	GENERATED_BODY()
	UPROPERTY() FSoftObjectPath Material;
	UPROPERTY() TMap<FName, FLinearColor> Vectors;
	UPROPERTY() TMap<FName, float> Scalars;
};

USTRUCT()
struct FLoopSavedMesh
{
	GENERATED_BODY()
	UPROPERTY() FName Component;
	UPROPERTY() bool bCustomDepth = false;
	UPROPERTY() int32 Stencil = 0;
	UPROPERTY() TArray<FLoopSavedMaterial> Materials;
};

USTRUCT()
struct FLoopSavedActor
{
	GENERATED_BODY()
	UPROPERTY() FName Name;
	UPROPERTY() FTransform Transform;
	UPROPERTY() TArray<uint8> Data;
	UPROPERTY() TArray<FLoopSavedMesh> Meshes;
};

UCLASS()
class MEGAJAM_2026_API ULoopSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() int32 Version = 1;
	UPROPERTY() FName Map;
	UPROPERTY() FTransform PlayerTransform;
	UPROPERTY() FRotator ControlRotation;
	UPROPERTY() FVector Gravity = FVector::DownVector;
	UPROPERTY() FVector Velocity;
	UPROPERTY() uint8 MovementMode = 3;
	UPROPERTY() bool bCrouched = false;
	UPROPERTY() bool bWorldColorRestored = false;
	UPROPERTY() FName SpawnPoint;
	UPROPERTY() TArray<FLoopSavedActor> Actors;
};
