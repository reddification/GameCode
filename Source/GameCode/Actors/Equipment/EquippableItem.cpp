#include "Actors/Equipment/EquippableItem.h"

#include "GameCode.h"
#include "Components/NameplateComponent.h"

AEquippableItem::AEquippableItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	NameplateComponent = CreateDefaultSubobject<UNameplateComponent>(TEXT("Nameplate"));
	NameplateComponent->SetupAttachment(RootComponent);
}

// SHIT OF SHITS but honestly 'clean' OOP doesn't look better

bool AEquippableItem::IsRangedWeapon() const
{
	return EquippableItemType == EEquippableItemType::AssaultRifle
		|| EquippableItemType == EEquippableItemType::OneHanded || EquippableItemType == EEquippableItemType::SniperRifle;
}

bool AEquippableItem::IsMeleeWeapon() const
{
	return EquippableItemType == EEquippableItemType::Melee;
}

bool AEquippableItem::IsThrowable() const
{
	return EquippableItemType == EEquippableItemType::Throwable;
}

void AEquippableItem::OnDropped()
{
	// TODO
	// WeaponMeshComponent->SetCollisionProfileName(ProfileWorldItem);
	// WeaponMeshComponent->SetSimulatePhysics(true);
}
