#pragma once

#include "CoreMinimal.h"

#include "GameCode/Actors/Interactive/Environment/Zipline.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"
#include "GameCode/Data/MantlingSettings.h"
#include "GameCode/Data/MontagePlayResult.h"
#include "GameFramework/Character.h"
#include "GCBaseCharacter.generated.h"

class UGCBaseCharacterMovementComponent;
class AInteractiveActor;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Controls")
	float BaseTurnRate = 45.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Controls")
	float BaseLookUpRate = 45.f;

public:
	virtual void Tick(float DeltaTime) override;

#pragma region INPUT
	
	void Interact();
	virtual void Turn(float Value) {}
	virtual void ToggleCrouchState();
	virtual void LookUp(float Value) {}
	virtual void TurnAtRate(float Value) {}
	virtual void LookUpAtRate(float Value) {}
	virtual void MoveForward(float Value);
	virtual void MoveRight(float Value) { CurrentInputRight = Value; }
	virtual void SwimForward(float Value){ CurrentInputForward = Value; }
	virtual void SwimRight(float Value) { CurrentInputRight = Value; }
	virtual void SwimUp(float Value) {  }
	virtual void Mantle(bool bForce = false);
	virtual void Jump() override;
	
	virtual void StartSprint();
	virtual void StopSprint();
	virtual void StopWallrun();
	virtual void StartWallrun();
	
	virtual void ClimbUp(float Value) { CurrentInputForward = Value; }
	virtual void ClimbDown(float Value) { CurrentInputForward = Value; }

	void StopZiplining();

#pragma endregion 
	
	const UGCBaseCharacterMovementComponent* GetGCMovementComponent () const { return GCMovementComponent; }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UInverseKinematicsComponent* InverseKinematicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	class ULedgeDetectionComponent* LedgeDetectionComponent; 

	virtual void OnJumped_Implementation() override;

	virtual bool CanCrouch() const override;
	virtual void OnStartCrouchOrProne(float HalfHeightAdjust);
	virtual void OnEndCrouchOrProne(float HalfHeightAdjust);

	void ToggleProneState();
	
	void RegisterInteractiveActor(const AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(const AInteractiveActor* InteractiveActor);

	float GetCurrentInputForward() const { return CurrentInputForward; }
	float GetCurrentInputRight() const { return CurrentInputRight; }

	void TryStartSliding();
	void StopSliding(bool bForce = false);

	bool IsMovementInputEnabled() const { return bMovementInputEnabled; }
	
protected:
	virtual bool CanSprint() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintStart();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintEnd();

#pragma region STAMINA

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxStamina = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StaminaRestoreVelocity = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SprintStaminaConsumptionRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float JumpStaminaConsumption = 10.f;

#pragma endregion STAMINA

#pragma region MANTLING
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling")
	FMantlingSettings MantleHighSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling")
	FMantlingSettings MantleLowSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling", meta=(ClampMin=0.f, UIMin=0.f))
	float MantleLowMaxHeight = 135.f;

#pragma endregion MANTLING

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Zipline")
	FName ZiplineHandPositionSocketName = FName("zipline_hand_position");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement|Hard Land")
	UAnimMontage* HardLandMontageTP = nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement|Hard Land")
	float HardLandHeight = 1000.f;
	
	virtual void OnSprintStart_Implementation();
	virtual void OnSprintEnd_Implementation();
	virtual void Falling() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyJumpApex() override;
	float CurrentInputForward = 0.f;
	float CurrentInputRight = 0.f;
	bool IsPendingMovement() const { return CurrentInputForward != 0 || CurrentInputRight != 0 && bMovementInputEnabled; }

	virtual void OnClimbableTopReached();
	virtual void OnStoppedClimbing(const AInteractiveActor* Interactable);
	virtual void OnZiplineObstacleHit(FHitResult Hit);
	bool CanMantle() const;

	virtual bool CanAttemptWallrun() const;

	UGCBaseCharacterMovementComponent* GCMovementComponent = nullptr;

	virtual bool CanJumpInternal_Implementation() const override;

	virtual void PlayMantleMontage(const FMantlingSettings& MantleSettings, float StartTime);
	virtual FMontagePlayResult PlayHardLandMontage() const;
	FMontagePlayResult PlayHardLandMontage(UAnimInstance* AnimInstance, UAnimMontage* AnimMontage) const;

	FOnMontageEnded MontageEndedUnlockControlsEvent;
	void OnMontageEndedUnlockControls(UAnimMontage* AnimMontage, bool bInterrupted);
	void SetInputDisabled(bool bDisabledMovement, bool bDisabledCamera);
	bool bMovementInputEnabled = true;
	
	const AInteractiveActor* CurrentInteractable = nullptr;
	
private:
	bool bSprintRequested = false;
	float CurrentStamina = 0.f;

	void TryChangeSprintState();
	bool CanRestoreStamina() const;
	bool IsConsumingStamina() const;
	float GetCurrentStaminaConsumption(float DeltaTime) const;
	void UpdateStamina(float DeltaTime);
	void ChangeStaminaValue(float StaminaModification);
	const FMantlingSettings& GetMantlingSettings(float Height) const;
	FZiplineParams GetZipliningParameters(const AZipline* Zipline) const;

	void TryStartInteracting();
	void TryStopInteracting();
	void ResetInteraction();
	const AInteractiveActor* GetPreferableInteractable();
	bool bInteracting = false;
	
	TArray<const AInteractiveActor*, TInlineAllocator<8>> InteractiveActors;

	bool CanSlide() const { return GCMovementComponent->IsSprinting(); }

	float FallApexWorldZ = 0.f;
};
