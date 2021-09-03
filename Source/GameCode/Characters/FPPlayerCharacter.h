// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "Controllers/GCPlayerController.h"
#include "FPPlayerCharacter.generated.h"

const FName SocketFPCamera = FName("CameraSocket");

UCLASS()
class GAMECODE_API AFPPlayerCharacter : public APlayerCharacter
{
	GENERATED_BODY()

public:
	AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnStartCrouchOrProne(float HalfHeightAdjust) override;
	virtual void OnEndCrouchOrProne(float HalfHeightAdjust) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual FRotator GetViewRotation() const override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|First person")
	class USkeletalMeshComponent* FirstPersonMeshComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|First person")
	class UCameraComponent* FirstPersonCameraComponent = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Climbing")
	float ClimbingCameraYawMin = -75.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Climbing")
	float ClimbingCameraYawMax = 75.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Climbing", meta=(UIMin=-60.f, UIMax = 0.f))
	float ClimbingCameraPitchMin = -60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Climbing", meta=(UIMin=0.f, UIMax = 60.f))
	float ClimbingCameraPitchMax = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Zipline")
	float ZipliningCameraYawMin = -75.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Zipline")
	float ZipliningCameraYawMax = 75.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Zipline", meta=(UIMin=-89.f, UIMax = 0.f))
	float ZipliningCameraPitchMin = -89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|First person|Zipline", meta=(UIMin=0.f, UIMax = 89.f))
	float ZipliningCameraPitchMax = 89.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement|Hard Land")
	UAnimMontage* HardLandMontageFP = nullptr;
	
	virtual void PlayMantleMontage(const FMantlingSettings& MantleSettings, float StartTime) override;
	virtual FMontagePlayResult PlayHardLandMontage() const override;
	
private:
	FOnMontageEnded OnMontageEnded;
	void OnMontageEndedCallback(UAnimMontage* AnimMontage, bool bInterrupted);
	TWeakObjectPtr<AGCPlayerController> GCPlayerController; 
	bool IsFPMontagePlaying() const;
	void RestrictCameraMovement(float PitchMin, float PitchMax, float YawMin, float YawMax, float YawAdjustment = 0.f);
};
