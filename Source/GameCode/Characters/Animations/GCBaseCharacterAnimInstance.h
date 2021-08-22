// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameCode/Data/GCMovementMode.h"
#include "GameCode/Data/Side.h"
#include "GCBaseCharacterAnimInstance.generated.h"

UCLASS()
class GAMECODE_API UGCBaseCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bInAir:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bCrouching:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bSprinting:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bProning:1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bOutOfStamina:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bSwimming:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bClimbingLadder:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bZiplining:1;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bWallRunning:1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESide WallrunSide = ESide::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 bSliding:1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character|IK")
	FVector RightFootEffectorLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character|IK")
	FVector LeftFootEffectorLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character|IK")
	FVector LeftLegJointTargetOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character|IK")
	FVector RightLegJointTargetOffset;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character|IK")
	FVector PelvisOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character|IK")
	FRotator LeftFootRotator = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character|IK")
	FRotator RightFootRotator = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ClimbingRatio = 0.0f;
	
	TWeakObjectPtr<class AGCBaseCharacter> Character;
};
