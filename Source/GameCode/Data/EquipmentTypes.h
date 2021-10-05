#pragma once

UENUM(BlueprintType)
enum class EAmmunitionType : uint8
{
	None = 0,
	Pistol = 1,
	AssaultRifle,
	Shotgun,
	SniperRifle,
	MAX  UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EReloadMode : uint8
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

UENUM(BlueprintType)
enum class EReloadType : uint8
{
	None = 0,
	FullClip = 1,
	ShellByShell = 2
};

UENUM(BlueprintType)
enum class EEquippableItemType : uint8
{
	None,
	OneHanded = 1,
	AssaultRifle,
	SniperRifle,
	Throwable
};

UENUM(BlueprintType)
enum class EThrowableType : uint8
{
	None = 0,
	Grenade = 1,
	MAX UMETA(Hidden)
};