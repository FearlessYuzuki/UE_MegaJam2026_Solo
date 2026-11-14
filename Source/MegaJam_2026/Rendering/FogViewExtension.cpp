#include "MegaJam_2026/Rendering/FogViewExtension.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "SceneInterface.h"
#include "SceneView.h"
#include "SceneViewExtension.h"

namespace
{
	TAutoConsoleVariable<float> FogViewDistance(
		TEXT("mj.FogViewDistance"),
		18000.0f,
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
			const UWorld* World = ViewFamily.Scene ? ViewFamily.Scene->GetWorld() : nullptr;
			const AWorldSettings* WorldSettings = World ? World->GetWorldSettings() : nullptr;
			const float FarDistance = FogViewDistance.GetValueOnGameThread();
			if (!View.bIsGameView || View.bIsSceneCapture || View.bIsReflectionCapture
				|| !View.IsPerspectiveProjection()
				|| !WorldSettings || !WorldSettings->ActorHasTag(TEXT("LimitedFogView"))
				|| !FMath::IsFinite(FarDistance) || FarDistance <= 0.0f)
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
	};
}

void RegisterFogViewExtension()
{
	static const TSharedRef<FFogViewExtension, ESPMode::ThreadSafe> Extension =
		FSceneViewExtensions::NewExtension<FFogViewExtension>();
}
