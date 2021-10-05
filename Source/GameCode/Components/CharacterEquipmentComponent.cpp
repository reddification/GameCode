#include "Components/CharacterEquipmentComponent.h"

#include "DrawDebugHelpers.h"
#include "Actors/Equipment/Throwables/ThrowableItem.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Actors/Projectiles/GCProjectile.h"
#include "Characters/GCBaseCharacter.h"
#include "Data/ReloadData.h"

void UCharacterEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	const auto Owner = GetOwner();
	checkf(Owner->IsA<AGCBaseCharacter>(), TEXT("CharacterEquipmentComponent is supposed to work only with AGCBaseCharacter derivatives"));
	CharacterOwner = StaticCast<AGCBaseCharacter*>(Owner); 
}

void UCharacterEquipmentComponent::CreateLoadout()
{
	Loadout.AddZeroed((uint32)EEquipmentSlot::MAX);
	Throwables.AddZeroed((uint32)EThrowableType::MAX);
	
	for (const TPair<EEquipmentSlot, TSubclassOf<ARangeWeaponItem>>& LoadoutSlot : InitialLoadoutTypes)
	{
		if (IsValid(LoadoutSlot.Value))
		{
			PickUpWeapon(LoadoutSlot.Key, LoadoutSlot.Value);
		}
	}

	for (const TPair<EThrowableType, TSubclassOf<AThrowableItem>>& ThrowableSlot : InitialThrowables)
	{
		if (IsValid(ThrowableSlot.Value))
		{
			PickUpThrowable(ThrowableSlot.Key, ThrowableSlot.Value);
		}
	}

	Pouch.AddZeroed((uint32)EAmmunitionType::MAX);
	for (const TPair<EAmmunitionType, int32> AmmoLimit : AmmunitionLimits)
	{
		Pouch[(uint32)AmmoLimit.Key] = AmmoLimit.Value > 0 ? AmmoLimit.Value : 0;
	}

	EquipWeapon(EEquipmentSlot::SideArm);
	EquipThrowable(EThrowableType::Grenade);
}

void UCharacterEquipmentComponent::PickUpWeapon(EEquipmentSlot Slot, const TSubclassOf<ARangeWeaponItem>& WeaponClass)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = GetOwner();
	ARangeWeaponItem* Weapon = GetWorld()->SpawnActor<ARangeWeaponItem>(WeaponClass, ActorSpawnParameters);
	Loadout[(uint32)Slot] = Weapon;
	WeaponPickedUpEvent.ExecuteIfBound(Weapon, true);
}

void UCharacterEquipmentComponent::PickUpThrowable(EThrowableType ThrowableType, const TSubclassOf<AThrowableItem>& ThrowableClass)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = GetOwner();
	AThrowableItem* Throwable = GetWorld()->SpawnActor<AThrowableItem>(ThrowableClass, ActorSpawnParameters);
	Throwable->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, ThrowablesPouchSocket);
	Throwables[(uint32)ThrowableType] = Throwable;
}

#pragma region CHANGING EQUIPMENT

void UCharacterEquipmentComponent::EquipWeapon(int delta)
{
	if (bChangingEquipment)
	{
		return;
	}
	
	uint8 CurrentSlotIndex = (uint8)EquippedSlot;
	uint8 NextSlotIndex = CurrentSlotIndex + delta;
	while (NextSlotIndex > (uint8)EEquipmentSlot::None && NextSlotIndex < (uint8)EEquipmentSlot::MAX && !IsValid(Loadout[NextSlotIndex]))
	{
		NextSlotIndex += delta;
	}

	if (NextSlotIndex != (uint8)EEquipmentSlot::None && NextSlotIndex != (uint8)EEquipmentSlot::MAX)
	{
		EquipWeapon((EEquipmentSlot)NextSlotIndex);
	}
}

// TODO perhaps equipping a weapon should be done in 2 phases: first unequip current weapon and only then start another timer to equip another weapon;

void UCharacterEquipmentComponent::EquipWeapon(EEquipmentSlot EquipmentSlot)
{
	if (bChangingEquipment || !IsValid(Loadout[(uint32)EquipmentSlot]))
	{
		return;
	}
	
	const auto NewWeapon = Loadout[(uint32)EquipmentSlot];
	if (IsValid(NewWeapon))
	{
		const float EquipmentDuration = NewWeapon->GetEquipmentDuration() * EquipDurationMultiplier;
		bChangingEquipment = true;

		ActiveEquippingData.OldItem = EquippedWeapon;
		ActiveEquippingData.NewItem = NewWeapon;
		ActiveEquippingData.Montage = NewWeapon->GetCharacterEquipMontage();
		ActiveEquippingData.EquipmentSlot = EquipmentSlot;
		ActiveEquippingData.bNotified = false;

		if (IsValid(ActiveEquippingData.Montage))
		{
			const float Duration = ActiveEquippingData.NewItem->GetEquipmentDuration() * EquipDurationMultiplier;
			ChangingEquippedItemStarted.ExecuteIfBound(ActiveEquippingData.Montage, Duration);
		}
		else
		{
		    GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2, FColor::Yellow,
		    	FString::Printf(TEXT("Changing weapon without animation in %fs"), EquipmentDuration), true);
		}
		
		GetWorld()->GetTimerManager().SetTimer(ChangingEquipmentTimer, this, &UCharacterEquipmentComponent::OnWeaponsChanged, EquipmentDuration);
	}
}

void UCharacterEquipmentComponent::EquipThrowable(EThrowableType ThrowableType)
{
	AThrowableItem* Throwable = Throwables[(uint32)ThrowableType];
	if (IsValid(Throwable))
	{
		EquippedThrowableSlot = ThrowableType;
		ActiveThrowable = Throwable;
		Throwable->AttachToComponent(CharacterOwner->GetMesh(),  FAttachmentTransformRules::KeepRelativeTransform,
			Throwable->GetCharacterUnequippedSocketName());
	}
}

void UCharacterEquipmentComponent::OnWeaponsChanged()
{
	if (WeaponEquippedChangedEvent.IsBound())
	{
		if (IsValid(ActiveEquippingData.OldItem))
		{
			WeaponEquippedChangedEvent.Broadcast(ActiveEquippingData.OldItem, false);
		}
		
		WeaponEquippedChangedEvent.Broadcast(ActiveEquippingData.NewItem, true);
	}
	
	OnAmmoChanged(ActiveEquippingData.NewItem->GetAmmo());
	ActiveEquippingData.bNotified = true;
	CompleteChangingWeapon();
}

void UCharacterEquipmentComponent::CompleteChangingWeapon()
{
	EquippedWeapon = ActiveEquippingData.NewItem;
	PreviousEquippedSlot = EquippedSlot;
	EquippedSlot = ActiveEquippingData.EquipmentSlot;
	// Could have added another event OnChangingWeaponsCompleted just to invoke this but there are so many events already I'm starting to be concerned
	CharacterOwner->UpdateStrafingControls();
	bChangingEquipment = false;
}

void UCharacterEquipmentComponent::InterruptChangingEquipment()
{
	if (!bChangingEquipment)
	{
		return;
	}

	bool bClearTimer = true;
	if (IsValid(ActiveEquippingData.Montage))
	{
		if (ActiveEquippingData.bNotified)
		{
			bClearTimer = false;
		}
	}

	if (bClearTimer)
	{
		GetWorld()->GetTimerManager().ClearTimer(ChangingEquipmentTimer);
	}
	
	bChangingEquipment = false;
}

#pragma endregion CHANGING EQUIPMENT

#pragma region RELOAD

FReloadData UCharacterEquipmentComponent::TryReload()
{
	if (bReloading || !IsValid(EquippedWeapon) || EquippedWeapon->GetEquippableItemType() == EEquippableItemType::None) return FReloadData();
	int32 ClipCapacity = EquippedWeapon->GetClipCapacity();
	int32 CurrentAmmo = EquippedWeapon->GetAmmo();
	if (CurrentAmmo == ClipCapacity) return FReloadData();
	int32& RemainingAmmo = Pouch[(uint32)EquippedWeapon->GetAmmunitionType()];
	if (RemainingAmmo <= 0) return FReloadData();
	
	bReloading = true;
	const float ReloadDuration = EquippedWeapon->GetReloadDuration() * ReloadDurationMultiplier;

	UAnimMontage* CharacterReloadMontage = EquippedWeapon->GetCharacterReloadMontage();
	if (EquippedWeapon->GetReloadType() != EReloadType::ShellByShell || !IsValid(CharacterReloadMontage))
	{
		GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UCharacterEquipmentComponent::CompleteReloading, ReloadDuration);
	}
	
	EquippedWeapon->StartReloading(ReloadDuration);
	return FReloadData(CharacterReloadMontage, ReloadDuration);
}

void UCharacterEquipmentComponent::InterruptReloading()
{
	if (!bReloading)
	{
		return;
	}
	
	bReloading = false;
	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->StopReloading(true);
		UAnimMontage* CharacterReloadMontage = EquippedWeapon->GetCharacterReloadMontage();
		if (IsValid(CharacterReloadMontage))
		{
			CharacterOwner->GetMesh()->GetAnimInstance()->Montage_Stop(CharacterReloadMontage->BlendOut.GetBlendTime(), CharacterReloadMontage);
		}
	}

	if (EquippedWeapon->GetReloadType() == EReloadType::FullClip)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	}
}

void UCharacterEquipmentComponent::CompleteReloading()
{
	if (!bReloading)
	{
		return;
	}
	
	bReloading = false;
	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->StopReloading(false);
	}

	int32 ClipCapacity = EquippedWeapon->GetClipCapacity();
	int32 CurrentAmmo = EquippedWeapon->GetAmmo();
	int32& RemainingAmmo = Pouch[(uint32)EquippedWeapon->GetAmmunitionType()];
	
	int32 AmmoToReload = 0;
	int32 NewAmmo = 0;
	switch (ReloadType)
	{
		case EReloadMode::KeepUnspentAmmo:
			AmmoToReload = FMath::Min(ClipCapacity - CurrentAmmo, RemainingAmmo);
			NewAmmo = CurrentAmmo + AmmoToReload;
			break;
		case EReloadMode::DiscardUnspendAmmo:
			NewAmmo = AmmoToReload = FMath::Min(ClipCapacity, RemainingAmmo);
			break;
		default:
			break;
	}

	RemainingAmmo -= AmmoToReload;
	EquippedWeapon->SetAmmo(NewAmmo);
}

void UCharacterEquipmentComponent::ReloadInsertShells(uint8 ShellsInsertedAtOnce)
{
	if (!bReloading)
	{
		return;
	}

	int32 ClipCapacity = EquippedWeapon->GetClipCapacity();
	int32 CurrentAmmo = EquippedWeapon->GetAmmo();
	int32& RemainingAmmo = Pouch[(uint32)EquippedWeapon->GetAmmunitionType()];
	int32 AmmoToReload = FMath::Min((int32)ShellsInsertedAtOnce, RemainingAmmo);
	RemainingAmmo -= AmmoToReload;
	int32 NewAmmo = CurrentAmmo + AmmoToReload;
	EquippedWeapon->SetAmmo(NewAmmo);
	if (NewAmmo == ClipCapacity)
	{
		UAnimInstance* CharacterAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = EquippedWeapon->GetMesh()->GetAnimInstance();
		CharacterAnimInstance->Montage_JumpToSection(ReloadMontageEndSectionName, EquippedWeapon->GetCharacterReloadMontage());
		WeaponAnimInstance->Montage_JumpToSection(ReloadMontageEndSectionName, EquippedWeapon->GetWeaponReloadMontage());
		bReloading = false;
	}
}

#pragma endregion RELOAD

#pragma region THROWING

bool UCharacterEquipmentComponent::TryThrow()
{
	if (ActiveThrowable.IsValid() && !bThrowing)
	{
		bThrowing = true;
		UAnimMontage* ThrowMontage = ActiveThrowable->GetThrowMontage();
		if (IsValid(ThrowMontage))
		{
			const float PlayRate = ThrowMontage->GetPlayLength() / ActiveThrowable->GetThrowDuration();
			EquippedWeapon->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SecondaryHandSocket);
			CharacterOwner->PlayAnimMontage(ThrowMontage, PlayRate);	
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(ThrowTimer, this, &UCharacterEquipmentComponent::ReleaseThrowableItem,
				ActiveThrowable->GetThrowDuration());
		}
		
		return true;
	}

	return false;
}

void UCharacterEquipmentComponent::GrabThrowableItem()
{
	if (!bThrowing)
	{
		return;
	}
	
	AGCProjectile* CurrentProjectile = ActiveThrowable->SpawnProjectile();
	CurrentProjectile->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, ThrowableHandSocket);
}

void UCharacterEquipmentComponent::InterruptThrowingItem()
{
	if (!bThrowing || !ActiveThrowable.IsValid())
	{
		return;
	}
	
	if (ActiveThrowable->GetThrowMontage())
	{
		CharacterOwner->StopAnimMontage(ActiveThrowable->GetThrowMontage());
	}
	
	GetWorld()->GetTimerManager().ClearTimer(ThrowTimer);
	ActiveThrowable->DropProjectile(CharacterOwner->GetController());
	bThrowing = false;
	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,
			EquippedWeapon->GetCharacterEquippedSocketName());
	}
}

void UCharacterEquipmentComponent::ActivateThrowableItem() const
{
	if (ActiveThrowable.IsValid())
	{
		ActiveThrowable->Activate(CharacterOwner->GetController());
	}
}

void UCharacterEquipmentComponent::ReleaseThrowableItem()
{
	if (!bThrowing || !ActiveThrowable.IsValid())
	{
		return;
	}

	ActiveThrowable->Throw(CharacterOwner->GetController());
	bThrowing = false;
	EquippedWeapon->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, EquippedWeapon->GetCharacterEquippedSocketName());
}

#pragma endregion THROWING

EEquippableItemType UCharacterEquipmentComponent::GetEquippedItemType() const
{
	return IsValid(EquippedWeapon) ? EquippedWeapon->GetEquippableItemType() : EEquippableItemType::None;
}

void UCharacterEquipmentComponent::StopFiring() const
{
	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->StopFiring();
	}
}

void UCharacterEquipmentComponent::OnAmmoChanged(int32 ClipAmmo) const
{
	if (IsValid(EquippedWeapon))
	{
		WeaponAmmoChangedEvent.ExecuteIfBound(ClipAmmo, Pouch[(uint32)EquippedWeapon->GetAmmunitionType()]);
	}
}
