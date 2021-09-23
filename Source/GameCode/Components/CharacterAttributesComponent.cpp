// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesComponent.h"

#include "DrawDebugHelpers.h"
#include "GameCode/GameCode.h"
#include "GameCode/GCDebugSubsystem.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "Kismet/GameplayStatics.h"

#if (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT)
#define ShowAttributes DebugDrawAttributes();
#else
#define ShowAttributes
#endif	

UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();
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
	
	ShowAttributes
}

void UCharacterAttributesComponent::UpdateOxygen(float DeltaTime)
{
	if (bSuffocating)
	{
		if (Oxygen > 0.f)
		{
			Oxygen = FMath::Max(Oxygen - OxygenConsumptionRate * DeltaTime, 0.f);
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
	else if (CanRestoreStamina())
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
	Stamina = FMath::Clamp(Stamina + StaminaModification, 0.f, MaxStamina);
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
	return (MovementMode != EMovementMode::MOVE_Walking && MovementMode != EMovementMode::MOVE_NavWalking) || bSprinting;
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
	if (HealthChangedEvent.IsBound())
	{
		HealthChangedEvent.Broadcast(Health / MaxHealth);
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


#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

void UCharacterAttributesComponent::DebugDrawAttributes() const
{
	if (!GetDebugSubsystem()->IsDebugCategoryEnabled(DebugCategoryAttributes))
	{
		return;
	}
	
	const float CapsuleOffset = 96.f * GetOwner()->GetActorScale().Z;
	const FVector TextLocation = GetOwner()->GetActorLocation() + (FVector::UpVector * CapsuleOffset);
	const float DrawStaminaOffset = 5.f;
	const float DrawHealthOffset = 10.f;
	const float DrawOxygenOffset = 15.f;

	DrawDebugString(GetWorld(), TextLocation + FVector::UpVector * DrawHealthOffset, FString::Printf(TEXT("Health: %.2f"), Health), nullptr,
		FColor::Red, 0.f, true);
	DrawDebugString(GetWorld(), TextLocation + FVector::UpVector * DrawStaminaOffset,
		FString::Printf(TEXT("Stamina: %.2f"), Stamina), nullptr, FColor::Yellow, 0.f, true);
	if (bSuffocating || Oxygen < MaxOxygen)
	{
		DrawDebugString(GetWorld(), TextLocation + FVector::UpVector * DrawOxygenOffset, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen), nullptr,
			FColor::Cyan, 0.f, true);
	}
}

const UGCDebugSubsystem* UCharacterAttributesComponent::GetDebugSubsystem() const
{
	if (!IsValid(DebugSubsystem))
	{
		DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UGCDebugSubsystem>();
	}

	return DebugSubsystem;
}

#endif	
