#pragma once
class ARangeWeaponItem;

struct FEquipmentData
{
	ARangeWeaponItem* OldItem;
	ARangeWeaponItem* NewItem;
	EEquipmentSlot EquipmentSlot;
	class UAnimMontage* Montage;
	bool bNotified;
};
