// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameCode/Data/MantlingMovementParameters.h"
#include "GameCode/Data/StopClimbingMethod.h"
#include "GameCode/Data/ZiplineParams.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GCBaseCharacterMovementComponent.generated.h"

DECLARE_DELEGATE_OneParam(FCrouchedOrProned, float HalfHeightAdjust)
DECLARE_DELEGATE_OneParam(FUncrouchedOrUnproned, float HalfHeightAdjust)
DECLARE_DELEGATE(FClimbableTopReached)
DECLARE_DELEGATE_OneParam(FStoppedClimbing, const class AInteractiveActor* Interactable)
DECLARE_DELEGATE_OneParam(FZiplineObstacleHit, FHitResult Hit);
class ALadder;

UCLASS()
class GAMECODE_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0, UIMin=0))
	float ProneCapsuleRadus = 40.0f;

	virtual void PhysicsRotation(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0, UIMin=0))
	float ProneCapsuleHalfHeight = 40.0f;
	
	bool IsSprinting() const {return bSprinting;}
	bool IsProning() const {return bProning;}
	bool IsStanding() const { return bProning || bWantsToCrouch; } 
	void StartSprint();
	void StopSprint(); 
	virtual float GetMaxSpeed() const override;
	void TryProne();
	void TryStandUp();
	void TryCrouch();
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;
	
	bool IsOutOfStamina() const { return bOutOfStamina; } 
	void SetIsOutOfStamina(bool bNewOutOfStamina);

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	FCrouchedOrProned CrouchedOrProned;
	FUncrouchedOrUnproned UncrouchedOrUnproned;
	
	bool TryStartMantle(const FMantlingMovementParameters& NewMantlingParameters);
	void EndMantle();
	bool IsMantling() const;
	float GetActorToClimbableProjection(const ALadder* Ladder, const FVector& ActorLocation) const;

	bool TryStartClimbing(const ALadder* Ladder);
	void StopClimbing(EStopClimbingMethod StopClimbingMethod = EStopClimbingMethod::Fall);
	bool IsClimbing() const;

	const ALadder* GetCurrentClimbable() const { return CurrentClimbable; }
	float GetClimbingSpeedRatio() const;

	FClimbableTopReached ClimbableTopReached;
	FStoppedClimbing StoppedClimbing;
	FZiplineObstacleHit ZiplineObstacleHit;
	
	bool IsZiplining() const;
	bool TryStartZiplining(const FZiplineParams& NewZiplineParams);
	void StopZiplining();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta=(ClampMin=0, UIMin=0))
	float SprintSpeed = 1200.f;

	float OutOfStaminaSpeed = 200.f;
	float DefaultWalkSpeed = 0.f;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY(Category="Character Movement|Swimming", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SwimmingCapsuleHalfHeight = 50.f;

	UPROPERTY(Category="Character Movement|Swimming", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SwimmingCapsuleRadius = 50.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MaxClimbingSpeed = 200.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingBrakingDeceleration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta=(ClampMin=0, UIMin=0))
	float CrawlSpeed = 200.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingOffset = 60.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingDetachTopOffset = 60.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingDetachBottomOffset = 90.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float JumpOffClimbableSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement|Zipline")
	float MinZiplineSpeed = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement|Zipline")
	float MaxZiplineSpeed = 1600.0f;
	
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;	
	
private:
	bool bSprinting = false;
	bool bOutOfStamina = false;

	bool bWantsToProne = false;
	bool bProning = false;

	void Prone();
	void UnProne();
	
	bool CanUnprone();
	bool CanProne();
	
	bool TryCrouchOrProne(float NewCapsuleHalfHeight, float NewCapsuleRadius);
	bool TryUncrouchOrUnprone();

	FMantlingMovementParameters MantlingParameters;
	FZiplineParams ZiplineParams;
	
	FTimerHandle MantlingTimerHandle;
	const ALadder* CurrentClimbable = nullptr;

	void PhysCustomMantling(float deltaTime, int32 Iterations);
	void PhysCustomClimbing(float deltaTime, int32 Iterations);
	void PhysCustomZiplining(float DeltaTime, int32 Iterations);
	
	TWeakObjectPtr<class AGCBaseCharacter> GCCharacter;

	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;
};
