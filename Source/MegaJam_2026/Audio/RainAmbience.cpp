#include "RainAmbience.h"

#include "Components/AudioComponent.h"
#include "Particles/Emitter.h"
#include "Particles/ParticleSystemComponent.h"

ARainAmbience::ARainAmbience()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	RainAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("RainAudio"));
	RootComponent = RainAudio;
	RainAudio->bAutoActivate = false;
	RainAudio->bAllowSpatialization = false;
	RainAudio->SetVolumeMultiplier(0.0f);
}

void ARainAmbience::BeginPlay()
{
	Super::BeginPlay();
	RainAudio->SetSound(RainSound);
}

void ARainAmbience::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	bool bRainActive = false;
	for (AEmitter* Emitter : RainEmitters)
	{
		if (!IsValid(Emitter) || Emitter->IsHidden())
		{
			continue;
		}
		const UParticleSystemComponent* Particles = Emitter->GetParticleSystemComponent();
		if (Particles && Particles->IsActive() && Particles->IsVisible() && !Particles->bWasDeactivated && !Particles->bDeactivateTriggered)
		{
			bRainActive = true;
			break;
		}
	}
	const float TargetVolume = bRainActive ? FMath::Max(Volume, 0.0f) : 0.0f;
	CurrentVolume = FMath::FInterpConstantTo(CurrentVolume, TargetVolume, DeltaSeconds, FMath::Max(Volume, 0.1f) * 2.0f);
	RainAudio->SetVolumeMultiplier(CurrentVolume);
	if (CurrentVolume > KINDA_SMALL_NUMBER && RainSound && !RainAudio->IsPlaying())
	{
		RainAudio->Play();
	}
	else if (CurrentVolume <= KINDA_SMALL_NUMBER && RainAudio->IsPlaying())
	{
		RainAudio->Stop();
	}
}
