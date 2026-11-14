#include "WorldsBasicActor.h"

#include "Characters/PlayerCharacter.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AWorldsBasicActor::AWorldsBasicActor()
{
	PrimaryActorTick.bCanEverTick = false;

	BaseBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseBox"));
	RootComponent = BaseBox;
	BaseBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		BaseBox->SetStaticMesh(CubeMesh.Object);
	}

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
	TriggerBox->SetGenerateOverlapEvents(true);
	// C++ 侧只给一份“初始摆放”，蓝图里的手动覆盖优先；持续同步交给 bAutoFitTriggerToBase
	TriggerBox->SetBoxExtent(TriggerExtent);
	if (const UStaticMesh* BaseMesh = BaseBox->GetStaticMesh())
	{
		TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f,
			BaseMesh->GetBoundingBox().Max.Z + TriggerHeightOffset));
	}
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AWorldsBasicActor::OnTriggerBoxBeginOverlap);
	TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AWorldsBasicActor::OnTriggerBoxEndOverlap);

	RefreshBaseShape();
}

void AWorldsBasicActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RefreshBaseShape();

	if (IdleMaterial && BaseBox)
	{
		BaseBox->SetMaterial(MaterialSlot, IdleMaterial);
	}
}

void AWorldsBasicActor::RefreshBaseShape()
{
	if (!BaseBox)
	{
		return;
	}

	const UStaticMesh* BaseMesh = BaseBox->GetStaticMesh();
	if (bAutoSizeBase && BaseMesh)
	{
		const FVector MeshSize = BaseMesh->GetBoundingBox().GetSize();
		const float Width = FMath::Max(BaseWidth, 10.0f);
		const float Height = FMath::Max(BaseHeight, 10.0f);
		if (MeshSize.X > KINDA_SMALL_NUMBER && MeshSize.Y > KINDA_SMALL_NUMBER
			&& MeshSize.Z > KINDA_SMALL_NUMBER)
		{
			BaseBox->SetRelativeScale3D(FVector(Width / MeshSize.X, Width / MeshSize.Y, Height / MeshSize.Z));
		}
	}

	if (TriggerBox && bAutoFitTriggerToBase && BaseMesh)
	{
		TriggerBox->SetBoxExtent(TriggerExtent);
		TriggerBox->SetRelativeLocation(FVector(0.0f, 0.0f,
			BaseMesh->GetBoundingBox().Max.Z + TriggerHeightOffset));
	}
}

void AWorldsBasicActor::BeginPlay()
{
	Super::BeginPlay();

	RefreshBaseShape();

	if (IdleMaterial && BaseBox)
	{
		BaseBox->SetMaterial(MaterialSlot, IdleMaterial);
	}
}

// ---------------------------------------------------------------------------
// 触发
// ---------------------------------------------------------------------------

void AWorldsBasicActor::OnTriggerBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                 bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled() || OverlappingPawns.Contains(Pawn))
	{
		return;
	}

	OverlappingPawns.Add(Pawn);

	UStaticMeshComponent* FootBlock = BaseBox;
	PawnFootBlocks.Add(Pawn, FootBlock);
	ApplyMaterialToBlock(FootBlock, true);

	if (bShowDebugMessages && GEngine)
	{
		const AActor* BlockOwner = IsValid(FootBlock) ? FootBlock->GetOwner() : nullptr;
		const FString BlockName = BlockOwner ? BlockOwner->GetActorNameOrLabel() : FString(TEXT("None"));
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
		                                 FString::Printf(TEXT("Player stepped on block: %s"), *BlockName));
	}

	BP_OnPlayerEntered(Pawn, FootBlock);
	OnPlayerOverlapChanged.Broadcast(Pawn, true);

	if (!ShouldFlipPlayerOnOverlap())
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());
	APlayerCharacter* Player = Cast<APlayerCharacter>(Pawn);
	if (MovementComponent && Player)
	{
		MovementComponent->SetGravityDirection(-MovementComponent->GetGravityDirection());
		Player->SetCameraAngle();
	}
}

bool AWorldsBasicActor::ShouldFlipPlayerOnOverlap() const
{
	return true;
}

void AWorldsBasicActor::OnTriggerBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || TriggerBox->IsOverlappingActor(Pawn))
	{
		return;
	}

	OverlappingPawns.Remove(Pawn);

	UStaticMeshComponent* FootBlock = nullptr;
	if (const TWeakObjectPtr<UStaticMeshComponent>* Found = PawnFootBlocks.Find(Pawn))
	{
		FootBlock = Found->Get();
	}
	PawnFootBlocks.Remove(Pawn);

	if (bRestoreOnEndOverlap)
	{
		// 只有当没有别的玩家还踩在同一方块上时才恢复
		bool bStillSteppedOn = false;
		for (const TPair<TWeakObjectPtr<APawn>, TWeakObjectPtr<UStaticMeshComponent>>& Pair : PawnFootBlocks)
		{
			if (Pair.Value.Get() == FootBlock)
			{
				bStillSteppedOn = true;
				break;
			}
		}

		if (!bStillSteppedOn)
		{
			RestoreBlock(FootBlock);
		}
	}

	if (bShowDebugMessages && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, TEXT("Player left the block"));
	}

	BP_OnPlayerExited(Pawn, FootBlock);
	OnPlayerOverlapChanged.Broadcast(Pawn, false);
}

void AWorldsBasicActor::SetFootBlockHighlighted(bool bHighlighted)
{
	if (bHighlighted)
	{
		for (const TPair<TWeakObjectPtr<APawn>, TWeakObjectPtr<UStaticMeshComponent>>& Pair : PawnFootBlocks)
		{
			ApplyMaterialToBlock(Pair.Value.Get(), true);
		}
	}
	else
	{
		RestoreAllBlocks();
	}
}

// ---------------------------------------------------------------------------
// 材质
// ---------------------------------------------------------------------------

void AWorldsBasicActor::ApplyMaterialToBlock(UStaticMeshComponent* Target, bool bHighlighted)
{
	if (!IsValid(Target))
	{
		return;
	}

	if (!bHighlighted)
	{
		RestoreBlock(Target);
		return;
	}

	// 首次改动前记录原始材质
	if (!OriginalMaterials.Contains(Target))
	{
		const int32 NumSlots = FMath::Max(Target->GetNumMaterials(), 1);
		TArray<TObjectPtr<UMaterialInterface>> Originals;
		Originals.Reserve(NumSlots);
		for (int32 Index = 0; Index < NumSlots; ++Index)
		{
			Originals.Add(Target->GetMaterial(Index));
		}
		OriginalMaterials.Add(Target, MoveTemp(Originals));
	}

	UMaterialInterface* SourceMaterial = OverlapMaterial;
	if (!SourceMaterial)
	{
		SourceMaterial = Target->GetMaterial(MaterialSlot);
	}

	UMaterialInterface* FinalMaterial = SourceMaterial;
	if (bUseOverlapTint && SourceMaterial)
	{
		if (UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(SourceMaterial, this))
		{
			FLinearColor UnusedColor;
			if (!SourceMaterial->GetVectorParameterValue(FMaterialParameterInfo(TintParameterName), UnusedColor))
			{
				UE_LOG(LogTemp, Warning,
				       TEXT("[%s] 材质 %s 没有名为 %s 的 Vector 参数，bUseOverlapTint 不会产生视觉变化。"),
				       *GetName(), *SourceMaterial->GetName(), *TintParameterName.ToString());
			}
			DynamicMaterial->SetVectorParameterValue(TintParameterName, OverlapTint);
			FinalMaterial = DynamicMaterial;
		}
	}

	if (FinalMaterial)
	{
		Target->SetMaterial(MaterialSlot, FinalMaterial);
	}
}

void AWorldsBasicActor::RestoreBlock(UStaticMeshComponent* Target)
{
	if (!IsValid(Target))
	{
		return;
	}

	if (TArray<TObjectPtr<UMaterialInterface>>* Originals = OriginalMaterials.Find(Target))
	{
		for (int32 Index = 0; Index < Originals->Num(); ++Index)
		{
			Target->SetMaterial(Index, (*Originals)[Index]);
		}
		OriginalMaterials.Remove(Target);
	}
}

void AWorldsBasicActor::RestoreAllBlocks()
{
	for (const TPair<TObjectPtr<UStaticMeshComponent>, TArray<TObjectPtr<UMaterialInterface>>>& Pair : OriginalMaterials)
	{
		UStaticMeshComponent* Target = Pair.Key;
		if (!IsValid(Target))
		{
			continue;
		}
		for (int32 Index = 0; Index < Pair.Value.Num(); ++Index)
		{
			Target->SetMaterial(Index, Pair.Value[Index]);
		}
	}
	OriginalMaterials.Empty();
}
