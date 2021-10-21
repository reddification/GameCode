// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"

#include "Components/CapsuleComponent.h"
#include "GameCode/GameCode.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	FirstPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPoleMesh"));
	FirstPoleStaticMeshComponent->SetupAttachment(RootComponent);
	SecondPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SecondPoleMesh"));
	SecondPoleStaticMeshComponent->SetupAttachment(RootComponent);
	CableStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CableMesh"));
	CableStaticMeshComponent->SetupAttachment(RootComponent);
	
	InteractionVolume = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractionVolumne"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(ProfileInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);
	UCapsuleComponent* InteractionCapsule = StaticCast<UCapsuleComponent*>(InteractionVolume);
	InteractionCapsule->SetCapsuleRadius(InteractionCapsuleRadius);

	FirstPoleStaticMeshComponent->bCastDynamicShadow = false;
	SecondPoleStaticMeshComponent->bCastDynamicShadow = false;
	CableStaticMeshComponent->bCastDynamicShadow = false;
}


void AZipline::Initialize()
{
	if (!IsValid(FirstPoleStaticMeshComponent->GetStaticMesh())
		|| !IsValid(SecondPoleStaticMeshComponent->GetStaticMesh())
		|| !IsValid(CableStaticMeshComponent->GetStaticMesh()))
	{
		return;
	}
	
	FirstPoleStaticMeshComponent->SetRelativeLocation(FirstPoleLocation);
	SecondPoleStaticMeshComponent->SetRelativeLocation(SecondPoleLocation);
	FirstPoleHalfHeight = FirstPoleStaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetExtent().Z;
	SecondPoleHalfHeight = SecondPoleStaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetExtent().Z;

	const float CablePoleOffsetRatio = 0.9f;
	AdjustedFirstPoleLocation = FirstPoleLocation + FVector::UpVector * FirstPoleHalfHeight * CablePoleOffsetRatio;
	AdjustedSecondPoleLocation = SecondPoleLocation + FVector::UpVector * SecondPoleHalfHeight * CablePoleOffsetRatio;
	const FVector CableLocation = (AdjustedFirstPoleLocation + AdjustedSecondPoleLocation) * 0.5f;
	const float CableLength = (AdjustedSecondPoleLocation - AdjustedFirstPoleLocation).Size();
	FVector ZiplineDirection = GetZiplineDirection();
	FRotator CableRotation = ZiplineDirection.ToOrientationRotator();
	CableRotation.Pitch += 90;
	CableStaticMeshComponent->SetRelativeLocation(CableLocation);
	const float DefaultCableLength = CableStaticMeshComponent->GetStaticMesh()->GetBoundingBox().GetSize().Z;
	CableStaticMeshComponent->SetRelativeScale3D(FVector(1, 1, CableLength / DefaultCableLength));
	CableStaticMeshComponent->SetRelativeRotation(CableRotation);
	
	UCapsuleComponent* InteractionCapsule = StaticCast<UCapsuleComponent*>(InteractionVolume);
	InteractionCapsule->SetCapsuleHalfHeight(CableLength * 0.5f);
	InteractionCapsule->SetRelativeLocation(CableLocation);
	InteractionCapsule->SetRelativeRotation(CableRotation);
	InteractionCapsule->SetCapsuleRadius(InteractionCapsuleRadius);
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	Initialize();
}

FVector AZipline::GetZiplineDirection() const
{
	const FVector& HigherPole = AdjustedFirstPoleLocation.Z > AdjustedSecondPoleLocation.Z
		? AdjustedFirstPoleLocation
		: AdjustedSecondPoleLocation;
	const FVector& LowerPole = AdjustedFirstPoleLocation.Z > AdjustedSecondPoleLocation.Z
		? AdjustedSecondPoleLocation
		: AdjustedFirstPoleLocation;

	return  LowerPole - HigherPole;
}

float AZipline::GetDeclinationAngle() const
{
	const float Hypotenuse = (AdjustedFirstPoleLocation - AdjustedSecondPoleLocation).Size();
	if (Hypotenuse == 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AZipline::GetDeclinationAngle Incorrect Zipline declination angle. Unpredictable Behaviour"));
		return 0.f;
	}
	
	const float Cathetus = FMath::Abs(AdjustedFirstPoleLocation.Z - AdjustedSecondPoleLocation.Z);
	const float DeclinationAngle = FMath::RadiansToDegrees(FMath::Asin(Cathetus / Hypotenuse)); 
	return DeclinationAngle;
}

FVector AZipline::GetTopPoleWorldLocation() const
{
	return (AdjustedFirstPoleLocation.Z > AdjustedSecondPoleLocation.Z
		? AdjustedFirstPoleLocation
		: AdjustedSecondPoleLocation) + GetActorLocation();
}

FVector AZipline::GetBottomPoleWorldLocation() const
{
	return (AdjustedFirstPoleLocation.Z > AdjustedSecondPoleLocation.Z
		? AdjustedSecondPoleLocation
		: AdjustedFirstPoleLocation) + GetActorLocation();
}

void AZipline::BeginPlay()
{
	Super::BeginPlay();
	Initialize();
}
