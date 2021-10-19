// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeDetectionComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameCode/GameCode.h"
#include "GameCode/GCDebugSubsystem.h"
#include "GameCode/GCGameInstance.h"
#include "GameCode/Utils/GCTraceUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void ULedgeDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
	
	checkf(GetOwner()->IsA<ACharacter>(), TEXT("LedgeDetectionComponent intended to be used with ACharacter derivatives"));
	CharacterOwner = StaticCast<ACharacter*>(GetOwner());
}

bool ULedgeDetectionComponent::DetectLedge(FLedgeDescriptor& LedgeDescriptor)
{
	const UCapsuleComponent* CharacterCapsule = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent();
	const float BottomZOffset = 2.f;
	const FVector CharacterBottom = CharacterOwner->GetActorLocation()
		- (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - BottomZOffset) * FVector::UpVector;
	
#if ENABLE_DRAW_DEBUG
	bool bDebugEnabled = GetDebugSubsystem()->IsDebugCategoryEnabled(DebugCategoryLedgeDetection);
#else
	bool bDebugEnabled = false;
#endif
	
	const GCTraceUtils::FTraceParams TraceParams(bDebugEnabled);

	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.bTraceComplex = true;
	CollisionQueryParams.AddIgnoredActor(CharacterOwner);
	
	const float ForwardCheckCapsuleRadius = CharacterCapsule->GetScaledCapsuleRadius();
	const float ForwardCheckCapsuleHalfHeight = (MaxLedgeHeight - MinLedgeHeight) * 0.5f;

	FVector ForwardSweepStartLocation = CharacterBottom + (MinLedgeHeight + ForwardCheckCapsuleHalfHeight) * FVector::UpVector;
	FVector ForwardSweepEndLocation = ForwardSweepStartLocation + CharacterOwner->GetActorForwardVector() * ForwardCheckDistance;
	
	FHitResult ForwardCheckHitResult;

	bool bForwardHit = GCTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), ForwardCheckHitResult,
		ForwardSweepStartLocation, ForwardSweepEndLocation, ForwardCheckCapsuleRadius, ForwardCheckCapsuleHalfHeight,
		ECC_Climbable, CollisionQueryParams, TraceParams);
	if (!bForwardHit)
	{
		return false;
	}

	FHitResult LedgeApproachHitResult;
	FVector LedgeApproachTraceStart = ForwardCheckHitResult.Location;
	LedgeApproachTraceStart.Z = CharacterBottom.Z + CharacterCapsule->GetScaledCapsuleHalfHeight() * 2 + CharacterCapsule->GetScaledCapsuleRadius();
	FVector LedgeApproachTraceEnd = CharacterBottom + BottomZOffset * FVector::UpVector;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	bool bCantApproachLedge = UKismetSystemLibrary::LineTraceSingle(this, LedgeApproachTraceStart,
    		LedgeApproachTraceEnd, TraceType,true,TArray<AActor*>(),
    		EDrawDebugTrace::None, LedgeApproachHitResult, true);

	if (bCantApproachLedge)
	{
		return false;
	}
	
	FHitResult DownwardCheckHitResult;
	const float DownwardSweepSphereRadius = ForwardCheckCapsuleRadius;
	float DownwardCheckDepthOffset = 15.f;
	FVector DownwardSweepStartLocation = ForwardCheckHitResult.ImpactPoint - ForwardCheckHitResult.ImpactNormal * DownwardCheckDepthOffset;
	DownwardSweepStartLocation.Z = CharacterBottom.Z + MaxLedgeHeight + ForwardCheckCapsuleRadius;	
	FVector DownwardSweepEndLocation = DownwardSweepStartLocation - FVector::UpVector * (MaxLedgeHeight - MinLedgeHeight);

	bool bDownwardHit = GCTraceUtils::SweepSphereSingleByChannel(GetWorld(), DownwardCheckHitResult,
		DownwardSweepStartLocation, DownwardSweepEndLocation, DownwardSweepSphereRadius, ECC_Climbable,
		CollisionQueryParams, TraceParams);
	
	if (!bDownwardHit)
	{
		return false;
	}

	const float OverlapCapsuleRadius = CharacterCapsule->GetScaledCapsuleRadius();
	const float OverlapCapsuleHalfHeight = CharacterCapsule->GetScaledCapsuleHalfHeight();
	FCollisionShape OverlapCapsuleShape = FCollisionShape::MakeCapsule(OverlapCapsuleRadius, OverlapCapsuleHalfHeight);
	FVector OverlapLocation = DownwardCheckHitResult.ImpactPoint + (OverlapCapsuleHalfHeight + BottomZOffset) * FVector::UpVector;
	
	bool bOverlap = GCTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, OverlapCapsuleRadius,
		OverlapCapsuleHalfHeight, ProfilePawn, CollisionQueryParams, TraceParams);

	if (bOverlap)
	{
		return false;
	}
	
	LedgeDescriptor.Location = OverlapLocation;
	LedgeDescriptor.Rotation = (ForwardCheckHitResult.ImpactNormal * FVector(-1.f, -1.f, 0)).ToOrientationRotator();
	LedgeDescriptor.LedgeNormal = ForwardCheckHitResult.ImpactNormal;
	LedgeDescriptor.MantlingHeight = DownwardCheckHitResult.Location.Z - CharacterBottom.Z;
	LedgeDescriptor.MantleTarget = DownwardCheckHitResult.GetActor();
	return true;
}

UGCDebugSubsystem* ULedgeDetectionComponent::GetDebugSubsystem()
{
	if (!IsValid(DebugSubsystem))
	{
		DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UGCDebugSubsystem>();
	}

	return DebugSubsystem;
}