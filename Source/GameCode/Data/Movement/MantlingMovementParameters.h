#pragma once
#include "MantlingSettings.h"
#include "Curves/CurveVector.h"
#include "GameCode/Components/Character/LedgeDetectionComponent.h"

struct FMantlingMovementParameters
{
	FMantlingMovementParameters()
	{
	}

	FMantlingMovementParameters(const FLedgeDescriptor& LedgeDescriptor, const FTransform& ActorTransform,
	                            const FMantlingSettings& MantlingSettings)
	{
		InitialLocation = ActorTransform.GetLocation();
		InitialRotation = ActorTransform.Rotator();
		TargetLocation = LedgeDescriptor.Location;

		MantleTarget = LedgeDescriptor.MantleTarget;
		InitialTargetLocation = IsValid(MantleTarget) ? MantleTarget->GetActorLocation() : LedgeDescriptor.Location;
		
		TargetRotation = LedgeDescriptor.Rotation;

		float MinRange, MaxRange;
		MantlingCurve = MantlingSettings.MantleCurve;
		MantlingSettings.MantleCurve->GetTimeRange(MinRange, MaxRange);

		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);
		StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange,
			LedgeDescriptor.MantlingHeight);
		Duration = MaxRange - MinRange;
		InitialAnimationLocation = TargetLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector;
		InitialAnimationLocation += MantlingSettings.AnimationCorrectionXY * LedgeDescriptor.LedgeNormal;
	}

	FVector InitialLocation = FVector::ZeroVector;
	FRotator InitialRotation = FRotator::ZeroRotator;

	FVector TargetLocation = FVector::ZeroVector;
	FRotator TargetRotation = FRotator::ZeroRotator;

	float Duration = 1.f;
	float StartTime = 0.f;

	FVector InitialAnimationLocation = FVector::ZeroVector;
	
	class UCurveVector* MantlingCurve;
	AActor* MantleTarget;
	FVector InitialTargetLocation;
};
