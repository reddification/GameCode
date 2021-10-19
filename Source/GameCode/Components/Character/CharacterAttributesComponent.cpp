// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesComponent.h"

#include "AIController.h"
#include "CharacterEquipmentComponent.h"
#include "DrawDebugHelpers.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "Kismet/GameplayStatics.h"

UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* Owner = GetOwner();
	checkf(Owner->IsA<AGCBaseCharacter>(), TEXT("UCharacterAttributesComponent is designed only for AGCBaseCharacter"));
	CharacterOwner = StaticCast<AGCBaseCharacter*>(Owner);
	Health = MaxHealth;
	Oxygen = MaxOxygen;
	Stamina = MaxStamina;
}

void UCharacterAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Health > 0.f)
	{
		UpdateOxygen(DeltaTime);
		UpdateStamina(DeltaTime);
	}
}

void UCharacterAttributesComponent::UpdateOxygen(float DeltaTime)
{
	bool bOxygenUpdated = false;
	if (bSuffocating)
	{
		if (Oxygen > 0.f)
		{
			Oxygen = FMath::Max(Oxygen - OxygenConsumptionRate * DeltaTime, 0.f);
			bOxygenUpdated = true;
		}
		else if (!GetWorld()->GetTimerManager().IsTimerActive(DrowningTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(DrowningTimer, this, &UCharacterAttributesComponent::DrowningCallback,
				DrowningHealthDamageInterval, true);
		}
	}
	else if (Oxygen < MaxOxygen)
	{
		Oxygen = FMath::Min(Oxygen + OxygenRestoreRate * DeltaTime, MaxOxygen);
		bOxygenUpdated = true;
	}

	if (bOxygenUpdated && AttributeChangedEvent.IsBound())
	{
		AttributeChangedEvent.Broadcast(ECharacterAttribute::Oxygen, Oxygen / MaxOxygen);
	}
}

void UCharacterAttributesComponent::DrowningCallback()
{
	const auto Controller = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	// Could have called OnTakeAnyDamage directly but still doing it through the character
	// just in case any other entities subscribed to AActor::OnTakeAnyDamage of the character
	GetOwner()->TakeDamage(DrowningHealthDamage, FDamageEvent(), Controller, GetOwner());
}

void UCharacterAttributesComponent::UpdateStamina(float DeltaTime)
{
	const float StaminaConsumption = GetStaminaConsumption();
	if (StaminaConsumption > 0.f)
	{
		ChangeStaminaValue(-StaminaConsumption * DeltaTime);
	}
	else if (CanRestoreStamina() && Stamina != MaxStamina)
	{
		ChangeStaminaValue(StaminaRestoreVelocity * DeltaTime);
	}
}

float UCharacterAttributesComponent::GetStaminaConsumption() const
{
	if (bSprinting || bWallrunning)
	{
		return SprintStaminaConsumptionRate;
	}
	return 0.f;
}

void UCharacterAttributesComponent::ChangeStaminaValue(float StaminaModification)
{
	if (FMath::IsNearlyZero(StaminaModification))
	{
		return;
	}
	
	Stamina = FMath::Clamp(Stamina + StaminaModification, 0.f, MaxStamina);
	if (AttributeChangedEvent.IsBound())
	{
		AttributeChangedEvent.Broadcast(ECharacterAttribute::Stamina, Stamina / MaxStamina);
	}
	
	if (Stamina == 0.f && !bRegeneratingStamina)
	{
		bRegeneratingStamina = true;
		if (OutOfStaminaEvent.IsBound())
		{
			OutOfStaminaEvent.Broadcast(true);
		}
	}
	else if (Stamina == MaxStamina && bRegeneratingStamina)
	{
		bRegeneratingStamina = false;
		if (OutOfStaminaEvent.IsBound())
		{
			OutOfStaminaEvent.Broadcast(false);
		}
	}
}

void UCharacterAttributesComponent::OnJumped()
{
	ChangeStaminaValue(-JumpStaminaConsumption);
}

bool UCharacterAttributesComponent::CanRestoreStamina() const
{
	return !bFalling && !IsConsumingStamina();
}

bool UCharacterAttributesComponent::IsConsumingStamina() const
{
	return (MovementMode != EMovementMode::MOVE_Walking && MovementMode != EMovementMode::MOVE_NavWalking)
		|| bSprinting
		|| CharacterOwner->GetEquipmentComponent()->IsMeleeAttacking();
}

void UCharacterAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
    AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}

	ChangeHealth(-Damage);
	if (Health <= 0.f && OutOfHealthEvent.IsBound())
	{
		OutOfHealthEvent.Broadcast();
	}
}

void UCharacterAttributesComponent::ChangeHealth(float Delta)
{
	Health = FMath::Clamp(Health + Delta, 0.f, MaxHealth);
	if (AttributeChangedEvent.IsBound())
	{
		AttributeChangedEvent.Broadcast(ECharacterAttribute::Health, Health / MaxHealth);
	}
}

void UCharacterAttributesComponent::SetSuffocating(bool bNewState)
{
	if (!IsAlive())
	{
		return;
	}
	
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	bSuffocating = bNewState;
	if (!bSuffocating && TimerManager.IsTimerActive(DrowningTimer))
	{
		TimerManager.ClearTimer(DrowningTimer);
	}
}