#include "Rendering/LoopWorldColorSubsystem.h"

#include "Engine/World.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"
#include "EngineUtils.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "GameFramework/PlayerController.h"

ULoopWorldColorSubsystem::ULoopWorldColorSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection> Collection(
		TEXT("/Game/MegaJam2026/Materials/PostProcess/MPC_WorldColor.MPC_WorldColor"));
	if (Collection.Succeeded()) ColorCollection = Collection.Object;
}

bool ULoopWorldColorSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void ULoopWorldColorSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	for (TActorIterator<AActor> Actor(&InWorld); Actor; ++Actor)
	{
		if (UExponentialHeightFogComponent* Fog = Actor->FindComponentByClass<UExponentialHeightFogComponent>())
			FogDensities.Emplace(Fog, Fog->FogDensity);
		if (UDirectionalLightComponent* Light = Actor->FindComponentByClass<UDirectionalLightComponent>())
			LightIntensities.Emplace(Light, Light->Intensity);
		TInlineComponentArray<UMeshComponent*> Meshes(*Actor);
		for (UMeshComponent* Mesh : Meshes)
		{
			for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
			{
				UMaterialInterface* Material = Mesh->GetMaterial(Index);
				if (Material && Material->GetPathName().Contains(TEXT("MI_FlipWorldDeepWater")))
					WaterMaterials.Add(Mesh->CreateDynamicMaterialInstance(Index));
			}
		}
	}
	RestoreSavedState(false);
}

void ULoopWorldColorSubsystem::ApplyColor(float Saturation)
{
	const APlayerController* Controller = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	const bool bOverview = Controller && Controller->GetViewTarget() && Controller->GetViewTarget()->ActorHasTag(TEXT("RingOverview"));
	for (auto& Fog : FogDensities)
		if (Fog.Key.IsValid()) Fog.Key->SetFogDensity(bOverview ? 0.00002f : Fog.Value * FMath::Lerp(1.0f, 0.12f, ColorProgress));
	for (auto& Light : LightIntensities)
		if (Light.Key.IsValid()) Light.Key->SetIntensity(Light.Value * FMath::Lerp(1.0f, 1.65f, ColorProgress));
	if (!ColorCollection || !GetWorld()) return;
	if (UMaterialParameterCollectionInstance* Instance = GetWorld()->GetParameterCollectionInstance(ColorCollection))
	{
		Instance->SetScalarParameterValue(TEXT("WorldColorRestored"), ColorProgress);
		Instance->SetScalarParameterValue(TEXT("RestorationSaturation"), Saturation);
	}
	for (UMaterialInstanceDynamic* Water : WaterMaterials)
	{
		if (!Water) continue;
		Water->SetScalarParameterValue(TEXT("Metallic"), FMath::Lerp(1.0f, 0.05f, ColorProgress));
		Water->SetVectorParameterValue(TEXT("Color Shallow"), FMath::Lerp(FLinearColor(0.23f, 0.25f, 0.28f), FLinearColor(0.015f, 0.55f, 0.7f), ColorProgress));
		Water->SetVectorParameterValue(TEXT("Color Deep"), FMath::Lerp(FLinearColor(0.055f, 0.065f, 0.08f), FLinearColor(0.008f, 0.12f, 0.32f), ColorProgress));
		Water->SetVectorParameterValue(TEXT("Color Coral Reef"), FMath::Lerp(FLinearColor(0.2f, 0.23f, 0.27f), FLinearColor(0.02f, 0.42f, 0.5f), ColorProgress));
	}
}

void ULoopWorldColorSubsystem::RestoreWorldColor()
{
	if (bWorldColorRestored) return;
	bWorldColorRestored = true;
	ColorProgress = 1.0f;
	PulseElapsed = 0.0f;
	ApplyColor(1.65f);
}

void ULoopWorldColorSubsystem::RestoreSavedState(bool bRestored)
{
	bWorldColorRestored = bRestored;
	ColorProgress = bRestored ? 1.0f : 0.0f;
	PulseElapsed = PulseDuration;
	ApplyColor(bRestored ? 1.3f : 1.0f);
}

void ULoopWorldColorSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PulseElapsed = FMath::Min(PulseElapsed + DeltaTime, PulseDuration);
	const float Fade = FMath::SmoothStep(0.0f, 1.0f, PulseElapsed / PulseDuration);
	ApplyColor(FMath::Lerp(1.65f, 1.3f, Fade));
}

bool ULoopWorldColorSubsystem::IsTickable() const
{
	return !IsTemplate() && bWorldColorRestored && PulseElapsed < PulseDuration;
}

TStatId ULoopWorldColorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULoopWorldColorSubsystem, STATGROUP_Tickables);
}

void ULoopWorldColorSubsystem::SetRestorationProgress(float Progress)
{
	ColorProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
	bWorldColorRestored = ColorProgress >= 1.0f;
	PulseElapsed = PulseDuration;
	ApplyColor(FMath::Lerp(1.0f, 1.3f, ColorProgress));
}
