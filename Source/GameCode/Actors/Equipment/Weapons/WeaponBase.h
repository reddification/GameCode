#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquippableItem.h"
// #include "WeaponBase.generated.h"


// TODO idea seems good but I honestly don't know a good way to implement it for all 3 weapon types and equipment component

// UCLASS()
// class GAMECODE_API AWeaponBase : public AEquipableItem
// {
// 	GENERATED_BODY()
//
// public:
// 	virtual void StartPrimaryAction(AController* Controller) { }
// 	virtual void StopPrimaryAction(bool bInterrupted) { }
// 	virtual void StartSecondaryAction(AController* Controller) { }
// 	virtual void StopSecondaryAction(bool bInterrupted = false) { }
// 	virtual void StartReloading() { }
// 	virtual void StopReloading(bool bInterrupted) { }
//
// 	virtual bool CanReload() { return true; }
// 	virtual bool CanDoPrimaryAction() { return true; }
// 	virtual bool CanDoSecondaryAction() { return true; }
//
// 	EWeaponType GetWeaponType() const { return WeaponType; }
// 	
// protected:
// 	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
// 	EWeaponType WeaponType = EWeaponType::None;
// };
