// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCode/Data/GCMovementMode.h"

#include "GameCode/Data/MantlingMovementParameters.h"
#include "GameCode/Data/Posture.h"
#include "GameCode/Data/Side.h"
#include "GameCode/Data/SlideData.h"
#include "GameCode/Data/SlideSettings.h"
#include "GameCode/Data/StopClimbingMethod.h"
#include "GameCode/Data/WakeUpParams.h"
#include "GameCode/Data/WallrunData.h"
#include "GameCode/Data/WallrunSettings.h"
#include "GameCode/Data/ZiplineParams.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GCBaseCharacterMovementComponent.generated.h"

DECLARE_DELEGATE_OneParam(FCrouchedOrProned, float HalfHeightAdjust)
DECLARE_DELEGATE_OneParam(FWokeUp, float HalfHeightAdjust)
DECLARE_DELEGATE(FClimbableTopReached)
DECLARE_DELEGATE_OneParam(FStoppedClimbing, const class AInteractiveActor* Interactable)
DECLARE_DELEGATE_OneParam(FWallrunBegin, const ESide WallrunSide)
DECLARE_DELEGATE_OneParam(FWallrunEnd, const ESide WallrunSide)
DECLARE_DELEGATE_OneParam(FZiplineObstacleHit, FHitResult Hit);
class ALadder;

UCLASS()
class GAMECODE_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	void InitPostureHalfHeights();
	virtual void BeginPlay() override;
	virtual void PhysicsRotation(float DeltaTime) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual bool CanAttemptJump() const override { return Super::CanAttemptJump() || IsSliding(); }
	bool TryStartSprint();
	void StopSprint(); 
	bool IsSprinting() const { return IsMovingOnGround() && bSprinting;}

	bool IsOutOfStamina() const { return bOutOfStamina; } 
	void SetIsOutOfStamina(bool bNewOutOfStamina);

#pragma region CROUCH/PRONE
	
	virtual bool IsCrouching() const override { return CurrentPosture == EPosture::Crouching; }
	bool IsProning() const { return CurrentPosture == EPosture::Proning; } 

	virtual float GetMaxSpeed() const override;
	void TryProne();
	void TryStandUp();
	void TryCrouch();
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;

	FCrouchedOrProned CrouchedOrProned;
	FWokeUp WokeUp;

#pragma endregion CROUCH/PRONE

	void SetForwardInput(float Value) { CurrentForwardInput = Value; }
	
	bool TryStartMantle(const FMantlingMovementParameters& NewMantlingParameters);
	void EndMantle();
	bool IsMantling() const;

#pragma region CLIMB

	float GetActorToClimbableProjection(const ALadder* Ladder, const FVector& ActorLocation) const;
	bool TryStartClimbing(const ALadder* Ladder);
	void StopClimbing(EStopClimbingMethod StopClimbingMethod = EStopClimbingMethod::Fall);
	bool IsClimbing() const;
	const ALadder* GetCurrentClimbable() const { return CurrentClimbable; }
	float GetClimbingSpeedRatio() const;
	FClimbableTopReached ClimbableTopReached;
	FStoppedClimbing StoppedClimbing;
	
#pragma endregion 

#pragma region ZIPLINE

	FZiplineObstacleHit ZiplineObstacleHit;
	bool IsZiplining() const;
	bool TryStartZiplining(const FZiplineParams& NewZiplineParams);
	void StopZiplining();
	const FZiplineParams& GetZiplineParams() const { return ZiplineParams; }

#pragma endregion ZIPLINE

	bool IsSliding() const;
	bool TryStartSliding();
	void StopSliding();
	void ResetFromSliding();
	void ResetSlide();

#pragma region WALLRUN

	void RequestWallrunning();
	void StopWallrunning(bool bResetTimer);
	bool IsWallrunning() const;
	ESide GetWallrunSide() const { return WallrunData.Side; }
	float GetMaxWallrunDuration () const { return WallrunSettings.MaxTime; }
	const FWallrunData& GetWallrunData() const { return WallrunData; }
	void JumpOffWall();

	FWallrunBegin WallrunBeginEvent;
	FWallrunEnd WallrunEndEvent;
	
#pragma endregion 

	const EPosture& GetCurrentPosture() const { return CurrentPosture; }
	
protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

#pragma region SPEED UPROPERTIES
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta=(ClampMin=0, UIMin=0))
	float SprintSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta=(ClampMin=0, UIMin=0))
	float OutOfStaminaSpeed = 200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Speed", meta=(ClampMin=0, UIMin=0))
	float CrawlSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement|Zipline")
	float MinZiplineSpeed = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement|Zipline")
	float MaxZiplineSpeed = 1600.0f;
	
#pragma endregion SPEED UPROPERTIES

#pragma region CAPSULE
	
	UPROPERTY(Category="Character Movement|Swimming", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SwimmingCapsuleHalfHeight = 50.f;

	UPROPERTY(Category="Character Movement|Swimming", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float SwimmingCapsuleRadius = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0, UIMin=0))
	float ProneCapsuleRadus = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0, UIMin=0))
	float ProneCapsuleHalfHeight = 40.0f;

#pragma endregion CAPSULE
	
#pragma region CLIMB

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float MaxClimbingSpeed = 200.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingBrakingDeceleration = 2048.0f;
	
	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingOffset = 60.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingDetachTopOffset = 60.f;

	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float ClimbingDetachBottomOffset = 90.f;
	
	UPROPERTY(Category="Character Movement|Climbing", EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", UIMin="0"))
	float JumpOffClimbableSpeed = 500.f;

#pragma endregion 

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWallrunSettings WallrunSettings;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FSlideSettings SlideSettings;
	
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;	
	
private:
	/* State that affects capsule half height */
	EPosture CurrentPosture = EPosture::Standing;
	float CurrentForwardInput = 0.f;
	float DefaultWalkSpeed = 0.f;

	bool bSprinting = false;
	bool bOutOfStamina = false;
	bool bWantsToProne = false;

	void Prone();
	void UnProne();
	bool CanUnprone();
	bool CanProne();
	
	bool TryCrouchOrProne(float NewCapsuleHalfHeight, float NewCapsuleRadius);
	void FillWakeUpParams(FWakeUpParams& WakeUpParams) const;
	bool TryWakeUpToState(EPosture DesiredPosture, bool bClientSimulation = false);
	bool TryWakeUp(float DesiredHalfHeight, const FWakeUpParams& WakeUpParams, bool bClientSimulation = false);

	bool CanApplyCustomRotation();
	
	FMantlingMovementParameters MantlingParameters;
	FZiplineParams ZiplineParams;
	FWallrunData WallrunData;
	FSlideData SlideData;
	
	FTimerHandle MantlingTimerHandle;
	const ALadder* CurrentClimbable = nullptr;

	void PhysCustomMantling(float DeltaTime, int32 Iterations);
	void PhysCustomClimbing(float DeltaTime, int32 Iterations);
	void PhysCustomWallRun(float DeltaTime, int32 iterations);
	void PhysCustomZiplining(float DeltaTime, int32 Iterations);
	void PhysCustomSliding(float DeltaTime, int32 Iterations);
	
	TWeakObjectPtr<class AGCBaseCharacter> GCCharacter;

	FRotator ForceTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;

	TTuple<FVector, ESide> GetWallrunBeginParams();
	FVector GetWallrunSurfaceNormal(const ESide& Side, const FVector& CharacterLocationDelta = FVector::ZeroVector) const;
	ESide GetWallrunSideFromNormal(const FVector& Normal) const;

	UFUNCTION()
	void OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	    FVector NormalImpulse, const FHitResult& Hit);

	bool IsSurfaceWallrunnable(const FVector& SurfaceNormal) const;

	mutable class UGCDebugSubsystem* DebugSubsystem;
	UGCDebugSubsystem* GetDebugSubsystem() const;
	bool IsInCustomMovementMode(const EGCMovementMode& Mode) const;

	EMovementMode GetMovementMode();
	
	TMap<EPosture, float> PostureCapsuleHalfHeights;
};