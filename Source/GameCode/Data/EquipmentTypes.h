#pragma once

UENUM(BlueprintType)
enum class EAmmunitionType : uint8
{
	None = 0,
	Pistol = 1,
	Rifle,
	MAX  UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EReloadType : uint8
{
	None = 0 UMETA(Hidden),
	KeepUnspentAmmo = 1,
	DiscardUnspendAmmo = 2
};

UENUM(BlueprintType)
enum class EEquipmentSlot : uint8
{
	None = 0,
	SideArm,
	Primary,
	MAX UMETA(Hidden)
};