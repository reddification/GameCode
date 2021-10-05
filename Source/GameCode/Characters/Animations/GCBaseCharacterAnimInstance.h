// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/EquipmentTypes.h"
#include "GameCode/Data/Side.h"
#include "GCBaseCharacterAnimInstance.generated.h"

UCLASS()
class GAMECODE_API UGCBaseCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Speed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCrouching;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSprinting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bProning;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bOutOfStamina;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSwimming;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bClimbingLadder;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bZiplining;	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bWallRunning;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ESide WallrunSide = ESide::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bSliding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bStrafing;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = -180, UIMax = 180))
	float Direction = 0.f;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquippableItemType EquippedItemType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character")
	FTransform WeaponForegripTransform;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character")
	bool bForegrip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Character")
	bool bAiming;
	
	TWeakObjectPtr<class AGCBaseCharacter> Character;
};
