// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBaseCharacter.h"
#include "Components/TimelineComponent.h"

#include "PlayerCharacter.generated.h"

/**
 * 
 */
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SprintSpringArmOffsetDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SprintSpringArmOffset = 420.f;

	void SetSpringArmPosition(float alpha) const;

	virtual void OnSprintStart_Implementation() override;
	virtual void OnSprintEnd_Implementation() override;

	virtual void OnStartCrouchOrProne(float HalfHeightAdjust) override;
	virtual void OnEndCrouchOrProne(float HalfHeightAdjust) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCurveFloat* SprintSpringArmTimelineCurve = nullptr;

private:
	FTimeline SprintSpringArmTimeline;

	float DefaultSpringArmOffset = 0.f;

	void InitSprintSpringArm();
	void AdjustSpringArm(const FVector& Adjustment);
	void AdjustSpringArmRelative(const FVector& Adjustment);
	void OnWallrunBegin(ESide Side);
	void OnWallrunEnd(ESide Side);
	void OnWallrunChanged(ESide Side, int AdjustmentModification);
};