#include "FootstepAudioComponent.h"

#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "MegaJam_2026/Actors/GroundBlock.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundSubmix.h"
#include "UObject/ConstructorHelpers.h"

UFootstepAudioComponent::UFootstepAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	ConstructorHelpers::FObjectFinder<USoundSubmix> Reverb(
		TEXT("/Game/MegaJam2026/Audio/Footsteps/SM_WaterFootstepReverb.SM_WaterFootstepReverb"));
	if (Reverb.Succeeded())
	{
		WaterReverbSubmix = Reverb.Object;
	}
	for (const FString& Category : {FString(TEXT("Water")), FString(TEXT("Grass")), FString(TEXT("Concrete"))})
	{
		TArray<TObjectPtr<USoundBase>>& Sounds = Category == TEXT("Water") ? WaterSounds
			: Category == TEXT("Grass") ? GrassSounds : ConcreteSounds;
		for (int32 Index = 1; Index <= 8; ++Index)
		{
			const FString Path = FString::Printf(TEXT("/Game/UltimateRealismFootsteps/%s/WAV/%s%d.%s%d"),
				*Category, *Category, Index, *Category, Index);
			ConstructorHelpers::FObjectFinder<USoundBase> Sound(*Path);
			if (Sound.Succeeded())
			{
				Sounds.Add(Sound.Object);
			}
		}
	}
}

void UFootstepAudioComponent::BeginPlay()
{
	Super::BeginPlay();
	Character = Cast<ACharacter>(GetOwner());
	if (!Character.IsValid())
	{
		SetComponentTickEnabled(false);
		return;
	}
	AddTickPrerequisiteComponent(Character->GetCharacterMovement());
	Character->LandedDelegate.AddDynamic(this, &UFootstepAudioComponent::HandleLanded);
	PreviousLocation = Character->GetActorLocation();
	for (TActorIterator<AActor> Actor(GetWorld()); Actor; ++Actor)
	{
		TInlineComponentArray<UStaticMeshComponent*> Meshes;
		Actor->GetComponents(Meshes);
		for (UStaticMeshComponent* Mesh : Meshes)
		{
			bool bWater = Actor->ActorHasTag(TEXT("Footstep.Water")) || Mesh->ComponentHasTag(TEXT("Footstep.Water"));
			for (UMaterialInterface* Material : Mesh->GetMaterials())
			{
				bWater |= IsWaterMaterial(Material);
			}
			if (bWater && Mesh->GetStaticMesh())
			{
				WaterSurfaces.Add(Mesh);
			}
		}
	}
}

bool UFootstepAudioComponent::IsWaterMaterial(const UMaterialInterface* Material)
{
	while (Material)
	{
		const FString Path = Material->GetPathName();
		if (Path.Contains(TEXT("/MegaJam2026/Materials/Instances/MI_FlipWorldDeepWater"))
			|| Path.Contains(TEXT("/Advanced_Water_Material/Materials/Water_Material/")))
		{
			return true;
		}
		const UMaterialInstance* Instance = Cast<UMaterialInstance>(Material);
		Material = Instance ? Instance->Parent.Get() : nullptr;
	}
	return false;
}

EFootstepSurface UFootstepAudioComponent::ClassifyMaterial(const UMaterialInterface* Material)
{
	if (IsWaterMaterial(Material))
	{
		return EFootstepSurface::Water;
	}
	while (Material)
	{
		const FString Path = Material->GetPathName();
		if (Path.Contains(TEXT("/Ground_Textures_v1/Materials/"))
			|| Path.Contains(TEXT("/UltraStylizedGrass/Materials/Ground/")))
		{
			return EFootstepSurface::Grass;
		}
		const UMaterialInstance* Instance = Cast<UMaterialInstance>(Material);
		Material = Instance ? Instance->Parent.Get() : nullptr;
	}
	return EFootstepSurface::Concrete;
}

bool UFootstepAudioComponent::IsOnWater(const FVector& FootLocation, const FVector& UpDirection) const
{
	for (const TWeakObjectPtr<UStaticMeshComponent>& Reference : WaterSurfaces)
	{
		const UStaticMeshComponent* Mesh = Reference.Get();
		if (!Mesh || !Mesh->IsVisible() || Mesh->bHiddenInGame || Mesh->GetOwner()->IsHidden() || !Mesh->GetStaticMesh())
		{
			continue;
		}
		const FTransform Transform = Mesh->GetComponentTransform();
		const FVector Normal = Transform.GetUnitAxis(EAxis::Z);
		if (FVector::DotProduct(Normal, UpDirection) < 0.5f)
		{
			continue;
		}
		const FVector LocalFoot = Transform.InverseTransformPosition(FootLocation);
		const FBox Bounds = Mesh->GetStaticMesh()->GetBoundingBox();
		const float SurfaceDistance = FVector::DotProduct(FootLocation - Transform.TransformPosition(
			FVector(LocalFoot.X, LocalFoot.Y, Bounds.Max.Z)), Normal);
		if (LocalFoot.X >= Bounds.Min.X && LocalFoot.X <= Bounds.Max.X
			&& LocalFoot.Y >= Bounds.Min.Y && LocalFoot.Y <= Bounds.Max.Y
			&& FMath::Abs(SurfaceDistance) <= 35.0f)
		{
			return true;
		}
	}
	return false;
}

EFootstepSurface UFootstepAudioComponent::GetCurrentSurface() const
{
	return Character.IsValid()
		? ResolveSurface(Character->GetCharacterMovement()->CurrentFloor.HitResult) : EFootstepSurface::Concrete;
}

EFootstepSurface UFootstepAudioComponent::ResolveSurface(const FHitResult& Hit) const
{
	if (!Character.IsValid())
	{
		return EFootstepSurface::Concrete;
	}
	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	const FVector UpDirection = -Movement->GetGravityDirection().GetSafeNormal();
	const FVector FootLocation = Character->GetActorLocation()
		- UpDirection * Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const UPrimitiveComponent* Component = Hit.GetComponent();
	const AActor* FloorActor = Hit.GetActor();
	for (const TPair<FName, EFootstepSurface>& Entry : {
		TPair<FName, EFootstepSurface>(TEXT("Footstep.Water"), EFootstepSurface::Water),
		TPair<FName, EFootstepSurface>(TEXT("Footstep.Grass"), EFootstepSurface::Grass),
		TPair<FName, EFootstepSurface>(TEXT("Footstep.Concrete"), EFootstepSurface::Concrete)})
	{
		if ((Component && Component->ComponentHasTag(Entry.Key)) || (FloorActor && FloorActor->ActorHasTag(Entry.Key)))
		{
			return Entry.Value;
		}
	}
	if (IsOnWater(FootLocation, UpDirection))
	{
		return EFootstepSurface::Water;
	}
	if (const AGroundBlock* GroundBlock = Cast<AGroundBlock>(FloorActor))
	{
		if (GroundBlock->IsActivated())
		{
			return EFootstepSurface::Grass;
		}
	}
	if (Component)
	{
		int32 Section = 0;
		const UMaterialInterface* Material = Hit.FaceIndex != INDEX_NONE
			? Component->GetMaterialFromCollisionFaceIndex(Hit.FaceIndex, Section) : Component->GetMaterial(0);
		return ClassifyMaterial(Material);
	}
	return EFootstepSurface::Concrete;
}

void UFootstepAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!Character.IsValid() || !Character->IsLocallyControlled())
	{
		return;
	}
	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	const FVector Location = Character->GetActorLocation();
	const FVector Displacement = Location - PreviousLocation;
	PreviousLocation = Location;
	const FVector Gravity = Movement->GetGravityDirection().GetSafeNormal();
	const float Speed = FVector::VectorPlaneProject(Movement->Velocity, Gravity).Size();
	TimeSinceStep += DeltaTime;
	if (Movement->IsMovingOnGround())
	{
		LastGroundSurface = GetCurrentSurface();
	}
	if (!Movement->IsMovingOnGround() || Speed < 15.0f || Character->IsMoveInputIgnored()
		|| Displacement.Size() > FMath::Max(200.0f, Speed * DeltaTime * 2.0f + 50.0f))
	{
		DistanceSinceStep = 0.0f;
		bWasWalking = false;
		return;
	}
	if (!bWasWalking)
	{
		DistanceSinceStep = FMath::Max(StepDistance, 30.0f) * 0.5f;
	}
	bWasWalking = true;
	DistanceSinceStep += FMath::Min(FVector::VectorPlaneProject(Displacement, Gravity).Size(), Speed * DeltaTime);
	if (DistanceSinceStep >= FMath::Max(StepDistance, 30.0f)
		&& TimeSinceStep >= FMath::Max(MinimumStepInterval, 0.05f))
	{
		DistanceSinceStep = 0.0f;
		TimeSinceStep = 0.0f;
		PlayFootstep();
	}
}

void UFootstepAudioComponent::PlayFootstep()
{
	if (PlaySurfaceSound(GetCurrentSurface(), EFootstepAudioEvent::Step,
		Character->bIsCrouched ? 0.65f : 1.0f, 1.0f))
	{
		++PlayedStepCount;
	}
}

void UFootstepAudioComponent::PlayJumpSound()
{
	if (!Character.IsValid() || !Character->IsLocallyControlled())
	{
		return;
	}
	if (PlaySurfaceSound(LastGroundSurface, EFootstepAudioEvent::Jump, 0.65f, 1.08f))
	{
		++PlayedJumpCount;
	}
	DistanceSinceStep = 0.0f;
	bWasWalking = false;
}

void UFootstepAudioComponent::HandleLanded(const FHitResult& Hit)
{
	if (!Character.IsValid() || !Character->IsLocallyControlled())
	{
		return;
	}
	const UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	const float ImpactSpeed = FVector::DotProduct(Movement->Velocity, Movement->GetGravityDirection().GetSafeNormal());
	if (ImpactSpeed < 180.0f)
	{
		return;
	}
	LastGroundSurface = ResolveSurface(Hit);
	const float ImpactGain = FMath::GetMappedRangeValueClamped(FVector2D(180.0f, 1200.0f), FVector2D(1.1f, 1.6f), ImpactSpeed);
	if (PlaySurfaceSound(LastGroundSurface, EFootstepAudioEvent::Landing, ImpactGain, 0.9f))
	{
		++PlayedLandingCount;
	}
	DistanceSinceStep = 0.0f;
	TimeSinceStep = 0.0f;
	bWasWalking = true;
}

USoundBase* UFootstepAudioComponent::PickSound(const TArray<TObjectPtr<USoundBase>>& Sounds) const
{
	TArray<USoundBase*, TInlineAllocator<8>> Candidates;
	for (USoundBase* Sound : Sounds)
	{
		if (IsValid(Sound) && Sound != PreviousSound.Get())
		{
			Candidates.Add(Sound);
		}
	}
	if (Candidates.IsEmpty())
	{
		for (USoundBase* Sound : Sounds)
		{
			if (IsValid(Sound))
			{
				Candidates.Add(Sound);
			}
		}
	}
	if (Candidates.IsEmpty())
	{
		return nullptr;
	}
	return Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
}

bool UFootstepAudioComponent::PlaySurfaceSound(EFootstepSurface Surface, EFootstepAudioEvent Event, float Gain, float Pitch)
{
	const float SurfaceVolume = Surface == EFootstepSurface::Water ? WaterVolume : Volume;
	if (SurfaceVolume <= 0.0f)
	{
		return false;
	}
	const TArray<TObjectPtr<USoundBase>>& Sounds = Surface == EFootstepSurface::Water ? WaterSounds
		: Surface == EFootstepSurface::Grass ? GrassSounds : ConcreteSounds;
	USoundBase* Sound = PickSound(Sounds);
	if (!Sound)
	{
		return false;
	}
	const float VolumeMultiplier = SurfaceVolume * Gain;
	LastFootstepAudio = UGameplayStatics::CreateSound2D(this, Sound, VolumeMultiplier,
		Pitch * FMath::FRandRange(0.96f, 1.04f));
	if (!LastFootstepAudio)
	{
		return false;
	}
	LastWaterGroundAudio = nullptr;
	if (Surface == EFootstepSurface::Water)
	{
		LastFootstepAudio->bEnableLowPassFilter = true;
		LastFootstepAudio->LowPassFilterFrequency = FMath::Clamp(WaterLowPassFrequency, 200.0f, 20000.0f);
		if (WaterReverbSubmix)
		{
			LastFootstepAudio->SetSubmixSend(WaterReverbSubmix, FMath::Clamp(WaterReverbSend, 0.0f, 1.0f));
		}
		if (WaterGroundLayerVolume > 0.0f)
		{
			if (USoundBase* GroundSound = PickSound(ConcreteSounds))
			{
				LastWaterGroundAudio = UGameplayStatics::CreateSound2D(this, GroundSound,
					VolumeMultiplier * FMath::Clamp(WaterGroundLayerVolume, 0.0f, 0.5f), Pitch);
				if (LastWaterGroundAudio)
				{
					LastWaterGroundAudio->bEnableLowPassFilter = true;
					LastWaterGroundAudio->LowPassFilterFrequency = 1800.0f;
					LastWaterGroundAudio->Play();
				}
			}
		}
	}
	LastFootstepAudio->Play();
	LastSurface = Surface;
	LastAudioEvent = Event;
	PreviousSound = Sound;
	return true;
}
