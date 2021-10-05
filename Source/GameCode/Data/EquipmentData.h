#pragma once

#include "EquipmentTypes.h"

class ARangeWeaponItem;

struct FEquipmentData
{
	ARangeWeaponItem* OldItem;
	ARangeWeaponItem* NewItem;
	EEquipmentSlot EquipmentSlot;
	class UAnimMontage* Montage;
	bool bNotified;
};
