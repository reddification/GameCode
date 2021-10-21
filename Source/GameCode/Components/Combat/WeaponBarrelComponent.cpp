#include "WeaponBarrelComponent.h"

AActor* UWeaponBarrelComponent::GetDamagingActor() const
{
	AActor* DamagingActor = GetOwner()->GetOwner();
	return IsValid(DamagingActor) ? DamagingActor : Super::GetDamagingActor();
}
