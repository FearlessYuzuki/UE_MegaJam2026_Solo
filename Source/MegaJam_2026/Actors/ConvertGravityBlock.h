// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "ConvertGravityBlock.generated.h"

UCLASS()
class MEGAJAM_2026_API AConvertGravityBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AConvertGravityBlock();
	
	/*
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Components")
	TObjectPtr<UBoxComponent> TriggerBox;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Components")
	TObjectPtr<UStaticMeshComponent> GravityBox;*/
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
