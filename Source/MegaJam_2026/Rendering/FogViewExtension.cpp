#include "MegaJam_2026/Rendering/FogViewExtension.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "SceneInterface.h"
#include "SceneView.h"
#include "SceneViewExtension.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/LoopWorldColorSubsystem.h"

namespace
{
	TAutoConsoleVariable<float> FogViewDistance(
		TEXT("mj.FogViewDistance"),
		25000.0f,
		TEXT("Perspective far clip distance in cm for worlds tagged LimitedFogView. 0 disables clipping."));

	class FFogViewExtension final : public FSceneViewExtensionBase
	{
	public:
		explicit FFogViewExtension(const FAutoRegister& AutoRegister)
			: FSceneViewExtensionBase(AutoRegister)
		{
		}

		virtual void SetupView(FSceneViewFamily& ViewFamily, FSceneView& View) override
		{
			UWorld* World = ViewFamily.Scene ? ViewFamily.Scene->GetWorld() : nullptr;
			const AWorldSettings* WorldSettings = World ? World->GetWorldSettings() : nullptr;
			if (!View.bIsGameView || View.bIsSceneCapture || View.bIsReflectionCapture
				|| !View.IsPerspectiveProjection()
				|| !WorldSettings || !WorldSettings->ActorHasTag(TEXT("LimitedFogView")))
			{
				return;
			}

			if (CachedWorld.Get() != World)
			{
				CachedWorld = World;
				TransitFogActors.Reset();
				for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
				{
					if (ActorIterator->ActorHasTag(TEXT("TransitFog")))
					{
						TransitFogActors.Add(*ActorIterator);
					}
				}
			}

			for (const TWeakObjectPtr<AActor>& FogReference : TransitFogActors)
			{
				const AActor* FogActor = FogReference.Get();
				if (!FogActor)
				{
					continue;
				}
				TInlineComponentArray<UPrimitiveComponent*> FogComponents;
				FogActor->GetComponents(FogComponents);
				for (const UPrimitiveComponent* FogComponent : FogComponents)
				{
					View.HiddenPrimitives.Add(FogComponent->GetPrimitiveSceneId());
				}
			}

			const APlayerController* Controller = World->GetFirstPlayerController();
			const ULoopWorldColorSubsystem* Color = World->GetSubsystem<ULoopWorldColorSubsystem>();
			if ((Controller && Controller->GetViewTarget() && Controller->GetViewTarget()->ActorHasTag(TEXT("RingOverview")))
				|| (Color && Color->IsWorldColorRestored())) return;
			const float FarDistance = FogViewDistance.GetValueOnGameThread();
			if (!FMath::IsFinite(FarDistance) || FarDistance <= 0.0f)
			{
				return;
			}

			FMatrix Projection = View.ProjectionMatrixUnadjustedForRHI;
			const double NearDistance = Projection.M[3][2] / (1.0 - Projection.M[2][2]);
			if (FarDistance <= NearDistance)
			{
				return;
			}

			Projection.M[2][2] = NearDistance / (NearDistance - FarDistance);
			Projection.M[3][2] = -FarDistance * NearDistance / (NearDistance - FarDistance);
			View.SceneViewInitOptions.OverrideFarClippingPlaneDistance = FarDistance;
			static bool bLogged = false;
			if (!bLogged)
			{
				UE_LOG(LogTemp, Display, TEXT("FogViewExtension applied: %.0f cm"), FarDistance);
				bLogged = true;
			}
			View.UpdateProjectionMatrix(Projection);
		}

	private:
		TWeakObjectPtr<UWorld> CachedWorld;
		TArray<TWeakObjectPtr<AActor>> TransitFogActors;
	};
}

void RegisterFogViewExtension()
{
	static const TSharedRef<FFogViewExtension, ESPMode::ThreadSafe> Extension =
		FSceneViewExtensions::NewExtension<FFogViewExtension>();
}
