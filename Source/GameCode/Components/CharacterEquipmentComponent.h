// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/CommonDelegates.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentData.h"
#include "Data/EquipmentTypes.h"
#include "Data/ReloadData.h"
#include "CharacterEquipmentComponent.generated.h"

// TODO too much logic in this component already. Perhaps move reload, throwables, shooting and meelee logic to a separate CombatComponent?

class ARangeWeaponItem;
class AThrowableItem;
DECLARE_DELEGATE_TwoParams(FWeaponPickedUpEvent, ARangeWeaponItem* EquippedWeapon, bool bPickedUp)
DECLARE_MULTICAST_DELEGATE_TwoParams(FWeaponEquippedChangedEvent, ARangeWeaponItem* EquippedWeapon, bool bEquipped)
// DECLARE_MULTICAST_DELEGATE_OneParam(FThrowableEquipStateChangedEvent, AThrowableItem* Throwable); // remove before submit
DECLARE_DELEGATE_TwoParams(FChangingEquippedItemStarted, UAnimMontage* Montage, float Duration)

// TODO 2 unused elements?
typedef TArray<int32, TInlineAllocator<(uint32)EAmmunitionType::MAX>> TAmmunitionArray;
typedef TArray<ARangeWeaponItem*, TInlineAllocator<(uint32)EEquipmentSlot::MAX>> TLoadoutArray;
typedef TArray<AThrowableItem*, TInlineAllocator<(uint32)EThrowableType::MAX>> TThrowablesArray;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	EEquippableItemType GetEquippedItemType() const;
	void StopFiring() const;

	bool IsPreferStrafing() const { return GetEquippedItemType() != EEquippableItemType::None || bThrowing; }

	ARangeWeaponItem* GetCurrentRangeWeapon() const { return EquippedWeapon; }
	FReloadData TryReload();
	void InterruptReloading();
	bool IsAutoReload() const { return bAutoReload; }

	mutable FWeaponPickedUpEvent WeaponPickedUpEvent;
	mutable FWeaponEquippedChangedEvent WeaponEquippedChangedEvent;
	mutable FWeaponAmmoChangedEvent WeaponAmmoChangedEvent;
	mutable FChangingEquippedItemStarted ChangingEquippedItemStarted;

	void CreateLoadout();

	void PickUpWeapon(EEquipmentSlot Slot, const TSubclassOf<ARangeWeaponItem>& WeaponClass);
	void EquipWeapon(EEquipmentSlot EquipmentSlot);
	void EquipThrowable(EThrowableType ThrowableType);

	void OnWeaponsChanged();
	void EquipWeapon(int delta);
	void InterruptChangingEquipment();

	void OnAmmoChanged(int32 ClipAmmo) const;
	bool IsChangingEquipment() const { return bChangingEquipment; }

	void ReloadInsertShells(uint8 ShellsInsertedAtOnce);

	void PickUpThrowable(EThrowableType ThrowableType, const TSubclassOf<AThrowableItem>& ThrowableClass);
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
	TMap<EEquipmentSlot, TSubclassOf<ARangeWeaponItem>> InitialLoadoutTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EThrowableType, TSubclassOf<AThrowableItem>> InitialThrowables;
	
	// The smaller this value the faster reloading goes
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
	FName ThrowableHandSocket = FName("hand_r_throwable");
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName SecondaryHandSocket = FName("hand_l_socket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sockets")
	FName ThrowablesPouchSocket = FName("throwables_pouch_socket");
	
private:
	// TODO TWeakObjectPtr too?
	ARangeWeaponItem* EquippedWeapon = nullptr;
	TWeakObjectPtr<AThrowableItem> ActiveThrowable = nullptr;

	EEquipmentSlot EquippedSlot = EEquipmentSlot::None;
	EEquipmentSlot PreviousEquippedSlot = EEquipmentSlot::None;
	EThrowableType EquippedThrowableSlot = EThrowableType::None;
	TWeakObjectPtr<class AGCBaseCharacter> CharacterOwner;

	void CompleteReloading();
	void CompleteChangingWeapon();
	
	TAmmunitionArray Pouch;
	TLoadoutArray Loadout;
	TThrowablesArray Throwables;
	
	bool bReloading = false;
	bool bChangingEquipment = false;
	bool bThrowing = false;
	
	FEquipmentData ActiveEquippingData;
		
	FTimerHandle ReloadTimer;
	FTimerHandle ChangingEquipmentTimer;
	FTimerHandle ThrowTimer;
};
