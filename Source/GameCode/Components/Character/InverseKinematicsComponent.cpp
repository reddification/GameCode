// Fill out your copyright notice in the Description page of Project Settings.


#include "InverseKinematicsComponent.h"
#include "Kismet/KismetSystemLibrary.h"

void UInverseKinematicsComponent::CalculateIkData(const USkeletalMeshComponent* SkeletalMesh, float CapsuleHalfHeight,
	FVector ActorLocation, bool bCrouched, float DeltaTime)
{
	UpdateLegsIkOffsetsBoxTrace(SkeletalMesh, CapsuleHalfHeight, ActorLocation, bCrouched, DeltaTime);
}

void UInverseKinematicsComponent::UpdateLegsIkOffsetsBoxTrace(const USkeletalMeshComponent* SkeletalMesh, float CapsuleHalfHeight,
	const FVector& ActorLocation, bool bCrouched, float DeltaSeconds)
{
	const float PreviousRightFootElevation = IkData.RightFootElevation;
	const float PreviousLeftFootElevation = IkData.LeftFootElevation;
	const float PreviousPelvisElevation = IkData.PelvisElevation;
	
	IkData.RightFootElevation = FMath::RoundToFloat(GetIkElevationForSocket(IkSettings.RightFootSocketName,
		SkeletalMesh, ActorLocation, CapsuleHalfHeight, bCrouched));
	IkData.LeftFootElevation = FMath::RoundToFloat(GetIkElevationForSocket(IkSettings.LeftFootSocketName,
		SkeletalMesh, ActorLocation, CapsuleHalfHeight, bCrouched));
	
	RecalculateFeetElevationsWithPelvis();

	if (IkData.RightFootElevation != PreviousRightFootElevation)
	{
		IkData.RightFootElevation = FMath::FInterpTo(PreviousRightFootElevation, IkData.RightFootElevation,
		DeltaSeconds, IkSettings.IKInterpSpeed);
	}
	if (IkData.LeftFootElevation != PreviousLeftFootElevation)
	{
		IkData.LeftFootElevation = FMath::FInterpTo(PreviousLeftFootElevation, IkData.LeftFootElevation,
			DeltaSeconds, IkSettings.IKInterpSpeed);
	}
	if (IkData.PelvisElevation != PreviousPelvisElevation)
	{
		IkData.PelvisElevation = FMath::FInterpTo(PreviousPelvisElevation, IkData.PelvisElevation,
		DeltaSeconds, IkSettings.IKInterpSpeed);
	}
	
	RecalculateFeetPitches(SkeletalMesh, DeltaSeconds);
	// IkData.LogFeetPivots();
	RecalculateKneesExtends(DeltaSeconds);
}

void UInverseKinematicsComponent::RecalculateFeetElevationsWithPelvis()
{
	// should be close to zero at the end of the day
	const float ElevationThreshold = -1;
	if (IkData.LeftFootElevation < ElevationThreshold)
	{
		IkData.PelvisElevation = IkData.LeftFootElevation;
		IkData.RightFootElevation += -IkData.LeftFootElevation;
		IkData.LeftFootElevation = 0;
	}
	else if (IkData.RightFootElevation < ElevationThreshold)
	{
		IkData.PelvisElevation = IkData.RightFootElevation;
		IkData.LeftFootElevation += -IkData.RightFootElevation;
		IkData.RightFootElevation = 0;
	}
	else
	{
		IkData.PelvisElevation = 0;
	}
}

// TODO just use surface normal you idiot
void UInverseKinematicsComponent::RecalculateFeetPitches(const USkeletalMeshComponent* SkeletalMesh, float DeltaSeconds)
{
	const float PreviousRightFootPitch = IkData.RightFootPitch;
	const float PreviousLeftFootPitch = IkData.LeftFootPitch;
	
	IkData.RightFootPitch = FMath::RoundToFloat(CalculateFootPitch(SkeletalMesh, IkSettings.RightHeelSocketName,
		IkSettings.RightFootSocketName, IkSettings.RightToesSocketName, PreviousRightFootPitch));
	IkData.LeftFootPitch = FMath::RoundToFloat(CalculateFootPitch(SkeletalMesh, IkSettings.LeftHeelSocketName,
		IkSettings.LeftFootSocketName, IkSettings.LeftToesSocketName, PreviousLeftFootPitch));
	
	IkData.RightFootPitch = FMath::FInterpTo(PreviousRightFootPitch, IkData.RightFootPitch,
		DeltaSeconds, IkSettings.IKInterpSpeed);

	IkData.LeftFootPitch = FMath::FInterpTo(PreviousLeftFootPitch, IkData.LeftFootPitch,
		DeltaSeconds, IkSettings.IKInterpSpeed);
}

void UInverseKinematicsComponent::RecalculateKneesExtends(float DeltaSeconds)
{
	const float PreviousRightKneeOutwardExtend = IkData.RightKneeOutwardExtend;
	const float PreviousLeftKneeOutwardExtend = IkData.LeftKneeOutwardExtend;
	IkData.RightKneeOutwardExtend = FMath::FInterpTo(PreviousRightKneeOutwardExtend,
		FMath::Lerp(0.f, IkSettings.MaxKneeOutwardExtend, IkData.RightFootElevation/IkSettings.MaxKneeOutwardExtend), DeltaSeconds,
		IkSettings.IKInterpSpeed);
	IkData.LeftKneeOutwardExtend = FMath::FInterpTo(PreviousLeftKneeOutwardExtend,
		FMath::Lerp(0.f, IkSettings.MaxKneeOutwardExtend, IkData.LeftFootElevation/IkSettings.MaxKneeOutwardExtend), DeltaSeconds,
		IkSettings.IKInterpSpeed);
}

float UInverseKinematicsComponent::GetIkElevationForSocket(const FName& SocketName, const USkeletalMeshComponent* SkeletalMesh,
	const FVector& ActorLocation, float CapsuleHalfHeight, bool bCrouched)
{
	const float TraceDistance = IkData.IKScale * bCrouched
		? IkSettings.IKTraceDistanceCrouch
		: IkSettings.IKTraceDistance;

	FTransform FootTransform = SkeletalMesh->GetSocketTransform(SocketName);
	FVector FootLocation = FootTransform.GetLocation();
	FRotator FootRotation = FootTransform.Rotator();
	if (FootRotation.Pitch < IkSettings.MinFootPitchForElevationTrace && !bCrouched)
	{
		return 0;
	}
	
	FVector TraceStart(FootLocation.X, FootLocation.Y, ActorLocation.Z - CapsuleHalfHeight + TraceDistance);
	FVector TraceEnd = TraceStart - (TraceDistance + IkSettings.TraceExtend) * FVector::UpVector;
	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	FVector FootHalfSize(1,  IkSettings.FootLength * 0.5f, IkSettings.FootWidth);
	
	bool bHit = UKismetSystemLibrary::BoxTraceSingle(this, TraceStart, TraceEnd, FootHalfSize,
		FootRotation, TraceType, true,TArray<AActor*>(),
		EDrawDebugTrace::None, HitResult, true);
	
	return bHit
		? (HitResult.Location.Z - (ActorLocation.Z - CapsuleHalfHeight)) / IkData.IKScale
		: 0;
}

float UInverseKinematicsComponent::CalculateFootPitch(const USkeletalMeshComponent* SkeletalMesh, const FName& HeelSocketName, const FName& FootSocketName,
	const FName& ToesSocketName, float PreviousValue)
{
	const FVector HeelLocation = SkeletalMesh->GetSocketLocation(HeelSocketName);
	const FVector ToesLocation = SkeletalMesh->GetSocketLocation(ToesSocketName);
	const FVector FootLocation = SkeletalMesh->GetSocketLocation(FootSocketName);
	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	
	bool bHeelHit = UKismetSystemLibrary::LineTraceSingle(this, HeelLocation,
		HeelLocation - FVector::UpVector * IkSettings.FootLength, TraceType,true,TArray<AActor*>(),
		EDrawDebugTrace::None, HitResult, true);
	float HeelDistance = bHeelHit ? HitResult.Location.Z - HeelLocation.Z : 0;

	bool bToesHit = UKismetSystemLibrary::LineTraceSingle(this, ToesLocation,
		ToesLocation - FVector::UpVector * IkSettings.FootLength, TraceType,true,TArray<AActor*>(),
		EDrawDebugTrace::None, HitResult, true);
	float ToesDistance = bToesHit ? HitResult.Location.Z - ToesLocation.Z : 0;

	bool bFootHit = UKismetSystemLibrary::LineTraceSingle(this, FootLocation,
		FootLocation - FVector::UpVector * IkSettings.FootLength, TraceType,true,TArray<AActor*>(),
		EDrawDebugTrace::None, HitResult, true);
	float FootDistance = bFootHit ? HitResult.Location.Z - FootLocation.Z : 0;

	if (!(bHeelHit || bToesHit || bFootHit))
		return 0;
	
	if (FMath::IsNearlyEqual(HeelDistance, FootDistance, 0.5f)
		|| FMath::IsNearlyEqual(ToesDistance, FootDistance, 0.5f))
	{
		return PreviousValue;
	}
	
	const float FeetElevationThreshold = -30.f;
	if (ToesDistance > FeetElevationThreshold * IkData.IKScale && HeelDistance > FeetElevationThreshold * IkData.IKScale)
	{
		float AngleAdjustmentFactor = 1.75; 
		float AtanRad = FMath::Atan(FMath::Abs(ToesDistance - HeelDistance)/(IkSettings.FootLength * IkData.IKScale));
		float FootDeclineAngle = FMath::RadiansToDegrees(AtanRad);
		float ClampedAngle = FMath::ClampAngle(FootDeclineAngle, 0, 30) * AngleAdjustmentFactor;
		return HeelDistance < ToesDistance ? -ClampedAngle : ClampedAngle;
	}
	
	return PreviousValue;
}


