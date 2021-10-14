// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAttributesComponent.h"
#include "Actors/CommonDelegates.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentData.h"
#include "Data/EquipmentTypes.h"
#include "Data/MeleeAttackData.h"
#include "Data/UserInterfaceTypes.h"
#include "CharacterEquipmentComponent.generated.h"

// TODO too much logic in this component already. Perhaps move reload, throwables, shooting and meelee logic to a separate CombatComponent?

class AEquippableItem;
class ARangeWeaponItem;
class AMeleeWeaponItem;
class AThrowableItem;

DECLARE_MULTICAST_DELEGATE_TwoParams(FWeaponEquippedEvent, const FText& Name, EReticleType Reticle)
DECLARE_MULTICAST_DELEGATE(FWeaponUnequippedEvent);
DECLARE_DELEGATE_TwoParams(FChangingEquippedItemStarted, UAnimMontage* Montage, float Duration)

DECLARE_MULTICAST_DELEGATE_TwoParams(FAimingStateChangedEvent, bool bAiming, ARangeWeaponItem* Weapon)
DECLARE_DELEGATE_OneParam(FAimingSpeedChangedEvent, float NewAimSpeed)
DECLARE_DELEGATE(FAimingSpeedResetEvent)
DECLARE_DELEGATE(FMeleeWeaponEquippedEvent)

// TODO 2 unused elements?
typedef TArray<int32, TInlineAllocator<(uint32)EAmmunitionType::MAX>> TAmmunitionArray;
typedef TArray<AEquippableItem*, TInlineAllocator<(uint32)EEquipmentSlot::MAX>> TLoadoutArray;
typedef TArray<AThrowableItem*, TInlineAllocator<(uint32)EThrowableType::MAX>> TThrowablesArray;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	EEquippableItemType GetEquippedItemType() const;
	
	void StartShooting(AController* Controller);
	void StopFiring() const;

	bool IsPreferStrafing() const;

	const ARangeWeaponItem* GetCurrentWeapon() const { return EquippedRangedWeapon; }
	const AMeleeWeaponItem* GetCurrentMeleeWeapon() const { return EquippedMeleeWeapon; }

	void TryReload();
	void InterruptReloading();
	bool IsAutoReload() const { return bAutoReload; }
	
	void StartPrimaryMeleeAttack(AController* AttackerController);
	void StopPrimaryMeleeAttack();
	void StartHeavyMeleeAttack(AController* AttackerController);
	void StopHeavyMeleeAttack();
	void StartMeleeAttack(EMeleeAttackType AttackType, AController* AttackerController);
	void StopMeleeAttack(EMeleeAttackType AttackType);
	void OnMeleeAttackCompleted();

	bool StartAiming();
	void StopAiming();
	bool IsAiming() const { return bAiming; }
	
	bool IsMeleeAttacking() const { return bMeleeAttack; }
	void SetMeleeHitRegEnabled(bool bEnabled);

	mutable FWeaponEquippedEvent WeaponEquippedEvent;
	mutable FWeaponUnequippedEvent WeaponUnequippedEvent;
	mutable FWeaponAmmoChangedEvent WeaponAmmoChangedEvent;
	mutable FChangingEquippedItemStarted ChangingEquippedItemStarted;
	mutable FThrowablesCountChanged ThrowablesCountChanged;
	mutable FThrowableEquippedEvent ThrowableEquippedEvent;
	mutable FAimingStateChangedEvent AimStateChangedEvent;
	mutable FAimingSpeedChangedEvent AimingSpeedChangedEvent;
	mutable FAimingSpeedResetEvent AimingSpeedResetEvent;
	mutable FMeleeWeaponEquippedEvent MeleeWeaponEquippedEvent;
	
	void CreateLoadout();

	void PickUpWeapon(EEquipmentSlot Slot, const TSubclassOf<AEquippableItem>& WeaponClass);
	void DropItem();
	void PickUpThrowable(EThrowableType ThrowableType, const TSubclassOf<AThrowableItem>& ThrowableClass);
	void EquipWeapon(EEquipmentSlot EquipmentSlot);
	void EquipThrowable(EThrowableType ThrowableType);

	void OnWeaponsChanged();
	void EquipWeapon(int delta);
	void InterruptChangingEquipment();
	bool CanReload();

	void OnAmmoChanged(int32 ClipAmmo) const;
	bool IsChangingEquipment() const { return bChangingEquipment; }

	void ReloadInsertShells(uint8 ShellsInsertedAtOnce);
	void StartTogglingFireMode();
	void InterruptTogglingFireMode();

	AEquippableItem* GetEquippedItem() const;
	bool TryThrow();
	void GrabThrowableItem();
	void InterruptThrowingItem();
	void ActivateThrowableItem() const;
	void ReleaseThrowableItem();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EAmmunitionType, int32> AmmunitionLimits;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EEquipmentSlot, TSubclassOf<AEquippableItem>> InitialLoadoutTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EThrowableType, TSubclassOf<AThrowableItem>> InitialThrowables;
	
	// The smaller this value the faster changing equipment goes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout")
	float EquipDurationMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reload")
	bool bAutoReload = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reload")
	EReloadMode ReloadType = EReloadMode::DiscardUnspendAmmo;

	// The smaller this value the faster reloading goes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reload")
	float ReloadDurationMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reload")
	FName ReloadMontageEndSectionName = FName("EndReload");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName SecondaryHandSocket = FName("hand_l_socket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName ThrowablesPouchSocket = FName("throwables_pouch_socket");

	// The smaller this value the faster melee attacks are
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Melee")
	float MeleeDurationMultiplier = 1.f;
	
private:
	// TODO TWeakObjectPtr too?
	ARangeWeaponItem* EquippedRangedWeapon = nullptr;
	AThrowableItem* ActiveThrowable = nullptr;
	AMeleeWeaponItem* EquippedMeleeWeapon = nullptr;

	EEquipmentSlot EquippedSlot = EEquipmentSlot::None;
	EEquipmentSlot PreviousEquippedSlot = EEquipmentSlot::None;
	EThrowableType EquippedThrowableSlot = EThrowableType::None;
	
	TWeakObjectPtr<class AGCBaseCharacter> CharacterOwner;

	void CompleteReloading();
	void CompleteChangingWeapon();
	void CompleteTogglingFireMode();

	TAmmunitionArray Pouch;
	TLoadoutArray Loadout;
	TThrowablesArray Throwables;
	
	bool bReloading = false;
	bool bChangingEquipment = false;
	bool bThrowing = false;
	bool bAiming = false;
	bool bMeleeAttack = false;
	
	FEquipmentData ActiveEquippingData;
		
	FTimerHandle ReloadTimer;
	FTimerHandle ChangingEquipmentTimer;
	FTimerHandle ThrowTimer;
	FTimerHandle ChangeFireModeTimer;
	FTimerHandle MeleeAttackTimer;
	
	void OnShot(UAnimMontage* Montage);
};
