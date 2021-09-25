// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Components/ActorComponent.h"
#include "Data/EquipmentData.h"
#include "Data/EquippableItemType.h"
#include "Data/EquipmentTypes.h"
#include "Data/ReloadData.h"
#include "CharacterEquipmentComponent.generated.h"

class ARangeWeaponItem;
DECLARE_DELEGATE_TwoParams(FWeaponPickedUpEvent, ARangeWeaponItem* EquippedWeapon, bool bPickedUp)
DECLARE_DELEGATE_TwoParams(FWeaponEquippedChangedEvent, ARangeWeaponItem* EquippedWeapon, bool bEquipped)
DECLARE_DELEGATE_TwoParams(FChangingEquippedItemStarted, UAnimMontage* Montage, float Duration)

// TODO 2 unused elements?
typedef TArray<int32, TInlineAllocator<(uint32)EAmmunitionType::MAX>> TAmmunitionArray;
typedef TArray<ARangeWeaponItem*, TInlineAllocator<(uint32)EEquipmentSlot::MAX>> TLoadoutArray;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	EEquippableItemType GetEquippedItemType() const;
	void StopFiring() const;

	bool IsPreferStrafing() const { return GetEquippedItemType() != EEquippableItemType::None; }

	ARangeWeaponItem* GetCurrentRangeWeapon() const { return EquippedWeapon; }
	FReloadData TryReload();
	void InterruptReloading();
	bool IsAutoReload() const { return bAutoReload; }

	mutable FWeaponPickedUpEvent WeaponPickedUpEvent;
	mutable FWeaponEquippedChangedEvent WeaponEquippedChangedEvent;
	mutable FWeaponAmmoChangedEvent WeaponAmmoChangedEvent;
	mutable FChangingEquippedItemStarted ChangingEquippedItemStarted;
	
	void CreateLoadout();
	
	void EquipItem(EEquipmentSlot EquipmentSlot);
	void OnItemsChanged() const;
	void EquipItem(int delta);
	void InterruptChangingEquipment();

	void OnAmmoChanged(int32 ClipAmmo) const;
	bool IsChangingEquipment() const { return bChangingEquipment; }

	void ReloadInsertShells(uint8 ShellsInsertedAtOnce);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EAmmunitionType, int32> AmmunitionLimits;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Loadout")
	TMap<EEquipmentSlot, TSubclassOf<ARangeWeaponItem>> InitialLoadoutTypes;

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
	
private:
	mutable ARangeWeaponItem* EquippedWeapon = nullptr;
	mutable EEquipmentSlot EquippedSlot = EEquipmentSlot::None;
	TWeakObjectPtr<class AGCBaseCharacter> CharacterOwner;

	void CompleteReloading();
	void CompleteChangingItem();
	
	TAmmunitionArray Pouch;
	TLoadoutArray Loadout;
	
	bool bReloading = false;
	bool bChangingEquipment = false;
	mutable FEquipmentData ActiveEquippingData;
		
	FTimerHandle ReloadTimer;
	FTimerHandle ChangingEquipmentTimer;
};
