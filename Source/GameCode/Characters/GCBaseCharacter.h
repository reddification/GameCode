#pragma once

#include "CoreMinimal.h"
#include "Actors/CommonDelegates.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Data/Movement/MantlingSettings.h"
#include "Data/Movement/ZiplineParams.h"

#include "GameCode/Actors/Interactive/Environment/Zipline.h"
#include "GameCode/Data/MontagePlayResult.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GCBaseCharacter.generated.h"

DECLARE_DELEGATE_OneParam(FAimingStateChangedEvent, bool bAiming);

class UGCBaseCharacterMovementComponent;
class UInverseKinematicsComponent;
class UCharacterEquipmentComponent;
class UCharacterAttributesComponent;
class AInteractiveActor;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter
{
	GENERATED_BODY()

friend UCharacterEquipmentComponent;
	
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

	UInverseKinematicsComponent* GetInverseKinematicsComponent() const { return InverseKinematicsComponent; }
	const UCharacterEquipmentComponent* GetEquipmentComponent () const { return CharacterEquipmentComponent; }
	const UGCBaseCharacterMovementComponent* GetGCMovementComponent () const { return GCMovementComponent; }
	const UCharacterAttributesComponent* GetCharacterAttributesComponent() const { return CharacterAttributesComponent; }
	
	mutable FAimingStateChangedEvent AimingStateChangedEvent;
	mutable FAmmoChangedEvent AmmoChangedEvent;
	
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
	
	virtual void StartRequestingSprint();
	virtual void StopRequestingSprint();
	virtual void StopRequestingWallrun();
	virtual void StartRequestingWallrun();
	
	virtual void ClimbUp(float Value) { CurrentInputForward = Value; }
	virtual void ClimbDown(float Value) { CurrentInputForward = Value; }

	void StopZiplining();

	bool CanShoot() const;
	void StartFiring();
	void StopFiring();
	void StartAiming();
	void StopAiming();
	void StartReloading();

	void PickWeapon(int Delta);
	
	bool CanAim() const;
	bool IsAiming() const { return bAiming; }
	
#pragma endregion 
	
	virtual void OnJumped_Implementation() override;

	virtual bool CanCrouch() const override;

	void ToggleProneState();
	
	void RegisterInteractiveActor(const AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(const AInteractiveActor* InteractiveActor);

	float GetCurrentInputForward() const { return CurrentInputForward; }
	float GetCurrentInputRight() const { return CurrentInputRight; }

	void TryStartSliding();
	void StopSliding(bool bForce = false);

	bool IsMovementInputEnabled() const { return bMovementInputEnabled; }
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Components")
	UInverseKinematicsComponent* InverseKinematicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class ULedgeDetectionComponent* LedgeDetectionComponent; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCharacterAttributesComponent* CharacterAttributesComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UCharacterEquipmentComponent* CharacterEquipmentComponent;
	
	virtual bool CanSprint() const;
	void TryStartSprinting();
	void StopSprinting();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintEnd();
	virtual void OnSprintEnd_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Aiming")
	void OnAimingStart(float FOV, float TurnModifier, float LookUpModifier);
	virtual void OnAimingStart_Implementation(float FOV, float TurnModifier, float LookUpModifier) {}

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Aiming")
	void OnAimingEnd();
	virtual void OnAimingEnd_Implementation() {}
	
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

	// Velocity.Z at which HardLand starts occuring
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Character|Movement|Hard Land")
	float HardLandVelocityZ = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName HeadBoneName = FName("head");
	
	virtual void Falling() override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void NotifyJumpApex() override;
	virtual void OnStartCrouchOrProne(float HalfHeightAdjust);
	virtual void OnEndCrouchOrProne(float HalfHeightAdjust);
	virtual void OnSlidingStateChangedEvent(bool bSliding, float HalfHeightAdjust);
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
	virtual FMontagePlayResult PlayHardLandMontage();
	FMontagePlayResult PlayHardLandMontage(UAnimInstance* AnimInstance, UAnimMontage* AnimMontage) const;

	FOnMontageEnded MontageEndedUnlockControlsEvent;
	void OnMontageEndedUnlockControls(UAnimMontage* AnimMontage, bool bInterrupted);
	void OnMantleEnded();
	void SetInputDisabled(bool bDisabledMovement, bool bDisabledCamera);
	bool bMovementInputEnabled = true;
	
	const AInteractiveActor* CurrentInteractable = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations")
	class UAnimMontage* DeathAnimationMontage;

	virtual void OnOutOfHealth();
	void StopWallrunning();
	virtual void OnOutOfStamina(bool bOutOfStamina);

	// X - absolute value of Velocity.Z, Y - Fall damage 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Attributes")
	UCurveFloat* FallDamageCurve = nullptr;

	void UpdateStrafingControls();
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	
	void OnWeaponEquippedChanged(class ARangeWeaponItem* EquippedWeapon, bool bEquipped);
	virtual void OnWeaponPickedUpChanged(class ARangeWeaponItem* EquippedWeapon, bool bPickedUp);

	bool bAiming = false;

	void InterruptReloading();
	bool CanReload() const;

	float PlayAnimMontageWithDuration(UAnimMontage* Montage, float DesiredDuration);
	
private:
	bool bSprintRequested = false;
	void TryChangeSprintState();
	
	const FMantlingSettings& GetMantlingSettings(float Height) const;
	FZiplineParams GetZipliningParameters(const AZipline* Zipline) const;
	void UpdateSuffocatingState();

	UAnimMontage* ActiveReloadMontage = nullptr;
	
	bool TryStartInteracting();
	void TryStopInteracting();
	void ResetInteraction();
	const AInteractiveActor* GetPreferableInteractable();
	bool bInteracting = false;

	TArray<const AInteractiveActor*, TInlineAllocator<8>> InteractiveActors;

	bool CanSlide() const { return GCMovementComponent->IsSprinting(); }

	float FallApexWorldZ = 0.f;

	void EnableRagdoll() const;

	void OnShot(UAnimMontage* AnimMontage);
	void OnChangingEquippedItemStarted(UAnimMontage* AnimMontage, float Duration);
	void ChangeMeshOffset(float HalfHeightAdjust);

	void OnWallrunEnd(ESide WallrunSide) const;
	void OnWallrunBegin(ESide WallrunSide);

	void SetStrafingControlsState(bool bStrafing);
};
