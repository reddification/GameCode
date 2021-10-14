#pragma once

#include "EquipmentTypes.h"

class AEquippableItem;

struct FEquipmentData
{
	AEquippableItem* OldItem;
	AEquippableItem* NewItem;
	EEquipmentSlot EquipmentSlot;
	class UAnimMontage* Montage;
	bool bNotified;
};
