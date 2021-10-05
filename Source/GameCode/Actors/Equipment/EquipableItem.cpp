#include "Actors/Equipment/EquipableItem.h"

#include "Components/NameplateComponent.h"

AEquipableItem::AEquipableItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	NameplateComponent = CreateDefaultSubobject<UNameplateComponent>(TEXT("Nameplate"));
	NameplateComponent->SetupAttachment(RootComponent);
}
