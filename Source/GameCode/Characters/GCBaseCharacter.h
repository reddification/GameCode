#pragma once

#include "CoreMinimal.h"

#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"
#include "GameCode/Data/MantlingSettings.h"
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

	const UGCBaseCharacterMovementComponent* GetGCMovementComponent () const { return GCMovementComponent; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UInverseKinematicsComponent* InverseKinematicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character|Movement")
	class ULedgeDetectionComponent* LedgeDetectionComponent; 

	virtual void OnJumped_Implementation() override;
	virtual bool CanJumpInternal_Implementation() const override;

	virtual bool CanCrouch() const override;
	virtual void OnStartCrouchOrProne(float HalfHeightAdjust);
	virtual void OnEndCrouchOrProne(float HalfHeightAdjust);

	void ToggleProneState();
	
	void RegisterInteractiveActor(const AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(const AInteractiveActor* InteractiveActor);

	float GetCurrentInputForward() const { return CurrentInputForward; }
	float GetCurrentInputRight() const { return CurrentInputRight; }
	
protected:
	virtual bool CanSprint() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintStart();

	UFUNCTION(BlueprintNativeEvent, Category = "Character|Movement")
	void OnSprintEnd();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxStamina = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StaminaRestoreVelocity = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SprintStaminaConsumptionRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float JumpStaminaConsumption = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling")
	FMantlingSettings MantleHighSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling")
	FMantlingSettings MantleLowSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character|Movement|Mantling", meta=(ClampMin=0.f, UIMin=0.f))
	float MantleLowMaxHeight = 135.f;
	
	virtual void OnSprintStart_Implementation();
	virtual void OnSprintEnd_Implementation();
	virtual void Falling() override;
	virtual void Landed(const FHitResult& Hit) override;
	float CurrentInputForward = 0.f;
	float CurrentInputRight = 0.f;
	bool IsPendingMovement() const { return CurrentInputForward != 0 || CurrentInputRight != 0; }

	virtual void OnClimbableTopReached();
	virtual void OnStoppedClimbing(const AInteractiveActor* Interactable);

	bool CanMantle() const;

	virtual bool CanAttemptWallrun() const;

	UGCBaseCharacterMovementComponent* GCMovementComponent = nullptr;

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

	void TryStopInteracting();
	void TryStartInteracting();
	bool bInteracting = false;
	
	const AInteractiveActor* CurrentInteractable = nullptr;
	TArray<const AInteractiveActor*, TInlineAllocator<8>> InteractiveActors;
};
