// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterAnimInstance.h"

#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/InverseKinematicsComponent.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGCBaseCharacterAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	APawn* Pawn = TryGetPawnOwner();
	checkf(Pawn->IsA<AGCBaseCharacter>(), TEXT("UGCBaseCharacterAnimInstance can be used only with an AGCBaseCharacter derivative"))
	Character = StaticCast<AGCBaseCharacter*>(Pawn);
}

void UGCBaseCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!Character.IsValid())
	{
		return;
	}

	const auto MovementComponent = Character->GetGCMovementComponent();
	Speed = MovementComponent->Velocity.Size();
	bInAir = MovementComponent->IsFlying() || MovementComponent->IsFalling();
	bCrouching = MovementComponent->IsCrouching();
	bSprinting = MovementComponent->IsSprinting();
	bOutOfStamina = MovementComponent->IsOutOfStamina();
	bProning = MovementComponent->IsProning();
	bSwimming = MovementComponent->IsSwimming();	
	bClimbingLadder = MovementComponent->IsClimbing();
	ClimbingRatio = bClimbingLadder ? MovementComponent->GetClimbingSpeedRatio() : 0.f;
	bWallRunning = MovementComponent->IsWallrunning();
	WallrunSide = MovementComponent->GetWallrunSide();
	bZiplining = MovementComponent->IsZiplining();
	bSliding = MovementComponent->IsSliding();
	
	const auto& IkData = Character->InverseKinematicsComponent->GetIkData();
	// constraining max foot elevation when crouching because it looks shitty with current animations
	float AdjustedRightFootElevation = bCrouching && IkData.RightFootElevation > 0.f
		? FMath::Clamp(IkData.RightFootElevation, 0.0f, 10.0f)
		: IkData.RightFootElevation;
	RightFootEffectorLocation = FVector(-AdjustedRightFootElevation, 0.f, 0.f);
	float AdjustedLeftFootElevation = bCrouching && IkData.LeftFootElevation > 0
		? FMath::Clamp(IkData.LeftFootElevation, 0.0f, 10.0f)
		: IkData.LeftFootElevation;
	LeftFootEffectorLocation = FVector(AdjustedLeftFootElevation, 0.f, 0.f); //left socket X facing upwards
	PelvisOffset = FVector(IkData.PelvisElevation, 0.f,0.f);

	const float FootMaxPitchInclination = Character->GetVelocity().Size() > 50.f ? 5.f : 30.f;
	
	//I guess animation blueprint should be better aware of some skeleton intricacies so adding extra constaints here as well
	RightFootRotator = FRotator(0.f, FMath::Clamp(IkData.RightFootPitch, -FootMaxPitchInclination, 40.f), 0.f);
	LeftFootRotator = FRotator(0.f, FMath::Clamp(IkData.LeftFootPitch, -FootMaxPitchInclination, 40.f), 0.f);

	RightLegJointTargetOffset = FVector(-IkData.RightKneeOutwardExtend, 0.f, 0.f);
	LeftLegJointTargetOffset = FVector(IkData.LeftKneeOutwardExtend * 0.75f, 0.f, 0.f);
}
