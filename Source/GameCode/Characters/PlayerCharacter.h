// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBaseCharacter.h"
#include "Components/TimelineComponent.h"

#include "PlayerCharacter.generated.h"

UCLASS(Blueprintable)
class GAMECODE_API APlayerCharacter : public AGCBaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void LookUp(float Value) override;
	virtual void Turn(float Value) override;
	virtual void LookUpAtRate(float Value) override;
	virtual void TurnAtRate(float Value) override;
	virtual void MoveForward(float Value) override;
	virtual void MoveRight(float Value) override;

	virtual void SwimForward(float Value) override;
	virtual void SwimRight(float Value) override;
	virtual void SwimUp(float Value) override;

	virtual void ClimbUp(float Value) override;
	virtual void ClimbDown (float Value) override;
	virtual void OnJumped_Implementation() override;
		
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* SpringArmComponent;

	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintEnd_Implementation() override;

	virtual void OnStartCrouchOrProne(float HalfHeightAdjust) override;
	virtual void OnEndCrouchOrProne(float HalfHeightAdjust) override;
	virtual void OnSlidingStateChangedEvent(bool bSliding, float HalfHeightAdjust) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|Movement|Sprint")
	float SprintSpringArmOffset = 420.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement|Sprint")
	UCurveFloat* SprintSpringArmTimelineCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Aiming")
	UCurveFloat* AimFovAdjustmentCurve = nullptr;

	virtual void OnAimingStart_Implementation(float FOV, float NewTurnModifier, float NewLookUpModifier) override;
	virtual void OnAimingEnd_Implementation() override;
	
private:
	FTimeline SprintSpringArmTimeline;
	FTimeline AimFovAdjustmentTimeline;
	
	float DefaultSpringArmOffset = 0.f;

	void InitTimeline(FTimeline& Timeline, UCurveFloat* Curve, void(APlayerCharacter::* Callback)(float) const);
	void AdjustSpringArm(const FVector& Adjustment);
	void AdjustSpringArmRelative(const FVector& Adjustment);
	void OnWallrunBegin(ESide Side);
	void OnWallrunEnd(ESide Side);
	void OnWallrunChanged(ESide Side, int AdjustmentModification);
	float DefaultFOV = 0.f;

	float TurnModifier = 1.f;
	float LookUpModifier = 1.f;
	float AimingFOV = 0.f;

	void SetAimFovPosition(float Alpha) const;
	void SetSpringArmPosition(float Alpha) const;

	TArray<FTimeline*> ActiveTimelines;
};