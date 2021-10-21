// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CharacterTypes.h"
#include "CharacterAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOutOfHealthEvent);
DECLARE_MULTICAST_DELEGATE_TwoParams(FAttributeChangedEvent, ECharacterAttribute Attribute, float Percent);
DECLARE_MULTICAST_DELEGATE_OneParam(FOutOfStaminaEvent, bool bOutOfStamina);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterAttributesComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	float GetStamina() const { return Stamina; }
	void SetFalling(bool bNewState) { bFalling = bNewState; }
	bool IsOutOfStamina() const { return bRegeneratingStamina; }
	void SetMovementMode(const TEnumAsByte<EMovementMode>& NewMovementMode) { MovementMode = NewMovementMode; }

	mutable FOutOfHealthEvent OutOfHealthEvent;
	mutable FOutOfStaminaEvent OutOfStaminaEvent;
	mutable FAttributeChangedEvent AttributeChangedEvent;
	
	void SetWallrunning(bool bState) { bWallrunning = bState; }
	void SetSprinting(bool bState) { bSprinting = bState; }

	bool IsAlive() const { return Health > 0.f; }

	void SetSuffocating(bool bNewState);

	void OnJumped();

	void ChangeStaminaValue(float StaminaModification);
	
	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(UIMin = 0.f))
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Oxygen", meta=(UIMin = 0.f))
	float MaxOxygen = 50.f;

	// Per second
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Oxygen", meta=(UIMin = 0.f))
	float OxygenRestoreRate = 15.0f;
	
	// Per second 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Oxygen", meta=(UIMin = 0.f))
	float OxygenConsumptionRate = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Oxygen", meta=(UIMin = 0.f))
	float DrowningHealthDamageInterval = 2.f;

	// Per DrowningHealthDamageInterval
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Oxygen", meta=(UIMin = 0.f))
	float DrowningHealthDamage = 5.f;

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
	
private:
	float Health = 0.f;
	float Oxygen = 50.f;

	bool bSuffocating = false;
	void UpdateOxygen(float DeltaTime);
	
	FTimerHandle DrowningTimer;
	void DrowningCallback();

	TWeakObjectPtr<class AGCBaseCharacter> CharacterOwner;
	float Stamina = 0.f;
	
	bool bWallrunning;
	bool bSprinting;
	bool bFalling;
	EMovementMode MovementMode;
	
	bool bRegeneratingStamina = false;
	
	bool CanRestoreStamina() const;
	bool IsConsumingStamina() const;
	float GetStaminaConsumption() const;
	void UpdateStamina(float DeltaTime);
	
	void ChangeHealth(float Delta);
	
};
