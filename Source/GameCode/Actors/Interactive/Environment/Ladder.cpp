// Fill out your copyright notice in the Description page of Project Settings.


#include "Ladder.h"

#include "Components/BoxComponent.h"
#include "GameCode/GameCode.h"
#include "GameFramework/Character.h"

ALadder::ALadder()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	LeftRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftRailMesh"));
	LeftRailMeshComponent->SetupAttachment(RootComponent);
	RightRailMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightRailMesh"));
	RightRailMeshComponent->SetupAttachment(RootComponent);
	StepMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StepMesh"));
	StepMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolumne"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(ProfileInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
	
	TopInteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TopInteractionVolumne"));
	TopInteractionVolume->SetupAttachment(RootComponent);
	TopInteractionVolume->SetCollisionProfileName(ProfileInteractionVolume);
	TopInteractionVolume->SetGenerateOverlapEvents(true);
}

void ALadder::BeginPlay()
{
	Super::BeginPlay();
	TopInteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &ALadder::OnInteractionVolumeOverlapBegin);
	TopInteractionVolume->OnComponentEndOverlap.AddDynamic(this, &ALadder::OnInteractionVolumeOverlapEnd);
}

void ALadder::OnConstruction(const FTransform& Transform)
{
	SetupLadderVerticalRail(LeftRailMeshComponent, -Width * 0.5f);
	SetupLadderVerticalRail(RightRailMeshComponent, Width * 0.5f);
	SetupStep();
	UBoxComponent* InteractionVolumeBox = GetLadderInteractionVolume();
	float BoxDepth = InteractionVolumeBox->GetUnscaledBoxExtent().X;
	InteractionVolumeBox->SetBoxExtent(FVector(BoxDepth, Width * 0.5f, Height * 0.5f));
	InteractionVolumeBox->SetRelativeLocation(FVector(BoxDepth, 0, Height * 0.5f));

	FVector TopBoxExtent = TopInteractionVolume->GetUnscaledBoxExtent();
	TopInteractionVolume->SetBoxExtent(FVector(TopBoxExtent.X, Width * 0.5f, TopBoxExtent.Z));
	TopInteractionVolume->SetRelativeLocation(FVector(-TopBoxExtent.X, 0.f, Height + TopBoxExtent.Z));
}

void ALadder::OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractionVolumeOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (!IsOverlappingCharacterCapsule(Cast<ACharacter>(OtherActor), OtherComp))
		return;
	
	if (OverlappedComponent == TopInteractionVolume)
	{
		bOnTop = true;
	}
}

void ALadder::OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnInteractionVolumeOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	if (!IsOverlappingCharacterCapsule(Cast<ACharacter>(OtherActor), OtherComp))
		return;
	
	if (OverlappedComponent == TopInteractionVolume)
	{
		bOnTop = false;
	}
}

void ALadder::SetupStep()
{
	UStaticMesh* StepsMesh = StepMeshComponent->GetStaticMesh();
	if (IsValid(StepsMesh))
	{
		float MeshWidth = StepsMesh->GetBoundingBox().GetSize().Y;
		if (MeshWidth > 0)
		{
			StepMeshComponent->SetRelativeScale3D(FVector(1, Width/MeshWidth, 1));
		}
	}
	StepMeshComponent->ClearInstances();
	uint32 StepsCount = FMath::FloorToInt((Height - FirstStepOffset) / StepsInterval);
	for (uint32 i = 0; i <= StepsCount; i++)
	{
		FTransform StepTransform(FVector(0, 0, FirstStepOffset + i * StepsInterval));
		StepMeshComponent->AddInstance(StepTransform);		
	}
}

UBoxComponent* ALadder::GetLadderInteractionVolume() const
{
	return StaticCast<UBoxComponent*>(InteractionVolume);
}

void ALadder::SetupLadderVerticalRail(UStaticMeshComponent* RailMeshComponent, float OffsetY)
{
	RailMeshComponent->SetRelativeLocation(FVector(0, OffsetY, Height * 0.5f));
	UStaticMesh* RailMesh = RailMeshComponent->GetStaticMesh();
	if (IsValid(RailMesh))
	{
		float MeshHeight = RailMesh->GetBoundingBox().GetSize().Z;
		if (MeshHeight > 0)
		{
			RailMeshComponent->SetRelativeScale3D(FVector(1, 1, Height / MeshHeight));
		}
	}
}

FVector ALadder::GetAttachFromTopAnimMontageStartingLocation() const
{
	// forward or up vector?
	FRotator OrientationRotation = GetActorForwardVector().ToOrientationRotator();
	FVector Offset = OrientationRotation.RotateVector(AttachFromTopAnimMontageInitialOffset);
	FVector LadderTop = GetActorLocation() + GetActorUpVector() * Height;
	return LadderTop + Offset;
}
