#include "Components/CharacterEquipmentComponent.h"

#include "CharacterAttributesComponent.h"
#include "DrawDebugHelpers.h"
#include "NameplateComponent.h"
#include "Actors/Equipment/Weapons/MeleeWeaponItem.h"
#include "Actors/Equipment/Weapons/ThrowableItem.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Actors/Projectiles/GCProjectile.h"
#include "Characters/GCBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// TODO CombatComponent after InventoryComponent

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
	
	for (const TPair<EEquipmentSlot, TSubclassOf<AEquippableItem>>& LoadoutSlot : InitialLoadoutTypes)
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

void UCharacterEquipmentComponent::PickUpWeapon(EEquipmentSlot Slot, const TSubclassOf<AEquippableItem>& WeaponClass)
{
	FActorSpawnParameters ActorSpawnParameters;
	ActorSpawnParameters.Owner = GetOwner();
	AEquippableItem* Weapon = GetWorld()->SpawnActor<AEquippableItem>(WeaponClass, ActorSpawnParameters);
	Loadout[(uint32)Slot] = Weapon;
	if (Weapon->IsRangedWeapon())
	{
		ARangeWeaponItem* RangeWeapon = StaticCast<ARangeWeaponItem*>(Weapon);
		RangeWeapon->ShootEvent.AddUObject(this, &UCharacterEquipmentComponent::OnShot);
		RangeWeapon->AmmoChangedEvent.BindUObject(this, &UCharacterEquipmentComponent::OnAmmoChanged);
		RangeWeapon->OutOfAmmoEvent.BindLambda([this](){ if (bAutoReload) TryReload(); });
	}

	Weapon->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, Weapon->GetCharacterUnequippedSocketName());
}

// TODO hmm
void UCharacterEquipmentComponent::DropItem()
{
	bool bItemDropped = false;
	AEquippableItem* ItemToDetach = nullptr;
	if (IsValid(EquippedRangedWeapon))
	{
		EquippedRangedWeapon->ShootEvent.RemoveAll(this);
		EquippedRangedWeapon->ShootEvent.RemoveAll(CharacterOwner.Get());
		EquippedRangedWeapon->OutOfAmmoEvent.Unbind();
		ItemToDetach = EquippedRangedWeapon;
		EquippedRangedWeapon = nullptr;
		bItemDropped = true;
	}
	else if (IsValid(EquippedMeleeWeapon))
	{
		ItemToDetach = EquippedMeleeWeapon;
		EquippedMeleeWeapon = nullptr;
		bItemDropped = true;
	}

	if (bItemDropped)
	{
		ItemToDetach->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		ItemToDetach->OnDropped();
		WeaponUnequippedEvent.Broadcast();
	}
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
	if (bChangingEquipment || !IsValid(Loadout[(uint32)EquipmentSlot]) || !CharacterOwner->CanStartAction(ECharacterAction::ChangeEquipment))
	{
		return;
	}
	
	const auto NewWeapon = Loadout[(uint32)EquipmentSlot];
	if (IsValid(NewWeapon))
	{
		const float EquipmentDuration = NewWeapon->GetEquipmentDuration() * EquipDurationMultiplier;
		bChangingEquipment = true;

		ActiveEquippingData.OldItem = GetEquippedItem();
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
		CharacterOwner->OnActionStarted(ECharacterAction::ChangeEquipment);
	}
}

void UCharacterEquipmentComponent::EquipThrowable(EThrowableType ThrowableType)
{
	AThrowableItem* Throwable = Throwables[(uint32)ThrowableType];
	if (IsValid(Throwable) && ThrowableEquippedEvent.IsBound())
	{
		int32 Count = Pouch[(uint32)Throwable->GetAmmunitionType()];
		EquippedThrowableSlot = ThrowableType;
		ActiveThrowable = Throwable;
		Throwable->AttachToComponent(CharacterOwner->GetMesh(),  FAttachmentTransformRules::KeepRelativeTransform,
			Throwable->GetCharacterUnequippedSocketName());
		
		ThrowableEquippedEvent.ExecuteIfBound(Throwable, Count);
	}
}

void UCharacterEquipmentComponent::OnWeaponsChanged()
{
	if (IsValid(ActiveEquippingData.OldItem))
	{
		// WeaponUnequippedEvent.Broadcast();
		ActiveEquippingData.OldItem->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,
			ActiveEquippingData.OldItem->GetCharacterUnequippedSocketName());
	}
	
	ActiveEquippingData.NewItem->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,
		ActiveEquippingData.NewItem->GetCharacterEquippedSocketName());
	
	if (ActiveEquippingData.NewItem->IsRangedWeapon())
	{
		EquippedMeleeWeapon = nullptr;
		ARangeWeaponItem* NewWeapon = StaticCast<ARangeWeaponItem*>(ActiveEquippingData.NewItem);
		EquippedRangedWeapon = NewWeapon;
		if (NewWeapon->CanAim())
		{
			AimingSpeedChangedEvent.ExecuteIfBound(NewWeapon->GetAimMaxSpeed());
		}
		else
		{
			AimingSpeedResetEvent.ExecuteIfBound();
		}
		
		OnAmmoChanged(NewWeapon->GetAmmo());
	}
	else if (ActiveEquippingData.NewItem->IsMeleeWeapon())
	{
		EquippedRangedWeapon = nullptr;
		EquippedMeleeWeapon = StaticCast<AMeleeWeaponItem*>(ActiveEquippingData.NewItem);
		AimingSpeedResetEvent.ExecuteIfBound();
		MeleeWeaponEquippedEvent.ExecuteIfBound();
	}
	
	ActiveEquippingData.bNotified = true;
	
	PreviousEquippedSlot = EquippedSlot;
	EquippedSlot = ActiveEquippingData.EquipmentSlot;
	CharacterOwner->UpdateStrafingControls();
	bChangingEquipment = false;
	if (WeaponEquippedEvent.IsBound())
	{
		WeaponEquippedEvent.Broadcast(ActiveEquippingData.NewItem->GetName(), ActiveEquippingData.NewItem->GetReticleType());
	}

	CharacterOwner->OnActionEnded(ECharacterAction::ChangeEquipment);
}

void UCharacterEquipmentComponent::InterruptChangingEquipment()
{
	if (!bChangingEquipment)
	{
		CharacterOwner->OnActionEnded(ECharacterAction::ChangeEquipment);
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
	
	CharacterOwner->OnActionEnded(ECharacterAction::ChangeEquipment);
	bChangingEquipment = false;
}

#pragma endregion CHANGING EQUIPMENT

#pragma region SHOOTING

bool UCharacterEquipmentComponent::StartAiming()
{
	if (!IsValid(EquippedRangedWeapon) || !EquippedRangedWeapon->CanAim() || !CharacterOwner->CanStartAction(ECharacterAction::Aim))
	{
		return false;
	}

	EquippedRangedWeapon->StartAiming();
	if (AimStateChangedEvent.IsBound())
	{
		AimStateChangedEvent.Broadcast(true, EquippedRangedWeapon);
	}

	bAiming = true;
	CharacterOwner->OnActionStarted(ECharacterAction::Aim);
	return true;
}

void UCharacterEquipmentComponent::StopAiming()
{
	CharacterOwner->OnActionEnded(ECharacterAction::Aim);
	if (!bAiming)
	{
		return;
	}
	
	bAiming = false;
	if (!IsValid(EquippedRangedWeapon))
	{
		return;
	}

	EquippedRangedWeapon->StopAiming();
	if (AimStateChangedEvent.IsBound())
	{
		AimStateChangedEvent.Broadcast(false, EquippedRangedWeapon);
	}
}

void UCharacterEquipmentComponent::StartShooting(AController* Controller)
{
	if (IsValid(EquippedRangedWeapon) && CharacterOwner->CanStartAction(ECharacterAction::Shoot))
	{
		bool bStarted = EquippedRangedWeapon->TryStartFiring(Controller);
		if (bStarted)
		{
			CharacterOwner->OnActionStarted(ECharacterAction::Shoot);
		}
	}
}

void UCharacterEquipmentComponent::StopFiring() const
{
	if (IsValid(EquippedRangedWeapon))
	{
		EquippedRangedWeapon->StopFiring();
		CharacterOwner->OnActionEnded(ECharacterAction::Shoot);
	}
}

void UCharacterEquipmentComponent::OnShot(UAnimMontage* Montage)
{
	CharacterOwner->PlayAnimMontage(Montage);
}

#pragma endregion SHOOTING

#pragma region RELOAD

bool UCharacterEquipmentComponent::CanReload()
{
	if (bReloading || !IsValid(EquippedRangedWeapon) || !EquippedRangedWeapon->IsRangedWeapon() || !CharacterOwner->CanStartAction(ECharacterAction::Reload))
	{
		return false;
	}

	int32 ClipCapacity = EquippedRangedWeapon->GetClipCapacity();
	int32 CurrentAmmo = EquippedRangedWeapon->GetAmmo();
	if (CurrentAmmo == ClipCapacity)
	{
		return false;
	}
	
	int32& RemainingAmmo = Pouch[(uint32)EquippedRangedWeapon->GetAmmunitionType()];
	if (RemainingAmmo <= 0)
	{
		return false;
	}
	
	return true;
}

void UCharacterEquipmentComponent::TryReload()
{
	if (!CanReload()) return;
	
	bReloading = true;
	CharacterOwner->OnActionStarted(ECharacterAction::Reload);
	const float ReloadDuration = EquippedRangedWeapon->GetReloadDuration() * ReloadDurationMultiplier;

	UAnimMontage* CharacterReloadMontage = EquippedRangedWeapon->GetCharacterReloadMontage();
	if (EquippedRangedWeapon->GetReloadType() != EReloadType::ShellByShell || !IsValid(CharacterReloadMontage))
	{
		GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UCharacterEquipmentComponent::CompleteReloading, ReloadDuration);
	}
	
	EquippedRangedWeapon->StartReloading(ReloadDuration);
	if (IsValid(CharacterReloadMontage))
	{
		CharacterOwner->PlayAnimMontageWithDuration(CharacterReloadMontage, ReloadDuration);
	}
}

void UCharacterEquipmentComponent::InterruptReloading()
{
	if (!bReloading)
	{
		CharacterOwner->OnActionEnded(ECharacterAction::Reload);
		return;
	}
	
	bReloading = false;
	if (IsValid(EquippedRangedWeapon))
	{
		EquippedRangedWeapon->StopReloading(true);
		UAnimMontage* CharacterReloadMontage = EquippedRangedWeapon->GetCharacterReloadMontage();
		if (IsValid(CharacterReloadMontage))
		{
			CharacterOwner->GetMesh()->GetAnimInstance()->Montage_Stop(CharacterReloadMontage->BlendOut.GetBlendTime(), CharacterReloadMontage);
		}
	}

	if (EquippedRangedWeapon->GetReloadType() == EReloadType::FullClip)
	{
		GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	}

	CharacterOwner->OnActionEnded(ECharacterAction::Reload);
}

void UCharacterEquipmentComponent::CompleteReloading()
{
	if (!bReloading)
	{
		CharacterOwner->OnActionEnded(ECharacterAction::Reload);
		return;
	}
	
	bReloading = false;
	if (IsValid(EquippedRangedWeapon))
	{
		EquippedRangedWeapon->StopReloading(false);
	}

	int32 ClipCapacity = EquippedRangedWeapon->GetClipCapacity();
	int32 CurrentAmmo = EquippedRangedWeapon->GetAmmo();
	int32& RemainingAmmo = Pouch[(uint32)EquippedRangedWeapon->GetAmmunitionType()];
	
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
	EquippedRangedWeapon->SetAmmo(NewAmmo);
	CharacterOwner->OnActionEnded(ECharacterAction::Reload);
}

void UCharacterEquipmentComponent::ReloadInsertShells(uint8 ShellsInsertedAtOnce)
{
	if (!bReloading)
	{
		CharacterOwner->OnActionEnded(ECharacterAction::Reload);
		return;
	}

	int32 ClipCapacity = EquippedRangedWeapon->GetClipCapacity();
	int32 CurrentAmmo = EquippedRangedWeapon->GetAmmo();
	int32& RemainingAmmo = Pouch[(uint32)EquippedRangedWeapon->GetAmmunitionType()];
	int32 AmmoToReload = FMath::Min((int32)ShellsInsertedAtOnce, RemainingAmmo);
	RemainingAmmo -= AmmoToReload;
	int32 NewAmmo = CurrentAmmo + AmmoToReload;
	EquippedRangedWeapon->SetAmmo(NewAmmo);
	if (NewAmmo == ClipCapacity)
	{
		UAnimInstance* CharacterAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
		UAnimInstance* WeaponAnimInstance = EquippedRangedWeapon->GetMesh()->GetAnimInstance();
		CharacterAnimInstance->Montage_JumpToSection(ReloadMontageEndSectionName, EquippedRangedWeapon->GetCharacterReloadMontage());
		WeaponAnimInstance->Montage_JumpToSection(ReloadMontageEndSectionName, EquippedRangedWeapon->GetWeaponReloadMontage());
		EquippedRangedWeapon->StopReloading(false);
		bReloading = false;
		CharacterOwner->OnActionEnded(ECharacterAction::Reload);
	}
}

#pragma endregion RELOAD

#pragma region FIRE MODES

void UCharacterEquipmentComponent::StartTogglingFireMode()
{
	if (!IsValid(EquippedRangedWeapon))
	{
		return;
	}

	if (!EquippedRangedWeapon->CanToggleFireMode() || bChangingEquipment || !CharacterOwner->CanStartAction(ECharacterAction::ToggleFireMode))
	{
		return;
	}

	const FFireModeSettings& FireModeSettings = EquippedRangedWeapon->GetNextFireModeSettings();
	GetWorld()->GetTimerManager().SetTimer(ChangeFireModeTimer, this, &UCharacterEquipmentComponent::CompleteTogglingFireMode, FireModeSettings.SwitchDuration);
	if (IsValid(FireModeSettings.SwitchMontage))
	{
		CharacterOwner->PlayAnimMontageWithDuration(FireModeSettings.SwitchMontage, FireModeSettings.SwitchDuration);
	}

	if (IsValid(FireModeSettings.ChangeSFX))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireModeSettings.ChangeSFX, EquippedRangedWeapon->GetActorLocation());
	}
	
	EquippedRangedWeapon->StartTogglingFireMode();
	CharacterOwner->OnActionStarted(ECharacterAction::ToggleFireMode);
}

void UCharacterEquipmentComponent::InterruptTogglingFireMode()
{
	if (!bChangingEquipment || !IsValid(EquippedRangedWeapon))
	{
		bChangingEquipment = false;
		CharacterOwner->OnActionEnded(ECharacterAction::ToggleFireMode);
	}

	const FFireModeSettings& FireModeSettings = EquippedRangedWeapon->GetNextFireModeSettings();
	GetWorld()->GetTimerManager().ClearTimer(ChangeFireModeTimer);
	if (IsValid(FireModeSettings.SwitchMontage))
	{
		CharacterOwner->StopAnimMontage(FireModeSettings.SwitchMontage);
	}
}

void UCharacterEquipmentComponent::CompleteTogglingFireMode()
{
	if (IsValid(EquippedRangedWeapon))
	{
		EquippedRangedWeapon->CompleteTogglingFireMode();
		CharacterOwner->OnActionEnded(ECharacterAction::ToggleFireMode);
	}
}

#pragma endregion FIRE MODES

#pragma region THROWING

bool UCharacterEquipmentComponent::TryThrow()
{
	if (!IsValid(ActiveThrowable) || bThrowing || !CharacterOwner->CanStartAction(ECharacterAction::ThrowItem))
	{
		return false;
	}
	
	int32 CurrentThrowableCount = Pouch[(uint32)ActiveThrowable->GetAmmunitionType()];
	if (CurrentThrowableCount <= 0)
	{
		return false;
	}
	
	bThrowing = true;
	CharacterOwner->OnActionStarted(ECharacterAction::ThrowItem);
	AEquippableItem* EquippedItem = GetEquippedItem();
	UAnimMontage* ThrowMontage = ActiveThrowable->GetThrowMontage();
	if (IsValid(ThrowMontage))
	{
		const float PlayRate = ThrowMontage->GetPlayLength() / ActiveThrowable->GetThrowDuration();
		if (IsValid(EquippedItem))
		{
			EquippedItem->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SecondaryHandSocket);
		}
		
		CharacterOwner->PlayAnimMontage(ThrowMontage, PlayRate);	
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(ThrowTimer, this, &UCharacterEquipmentComponent::ReleaseThrowableItem,
			ActiveThrowable->GetThrowDuration());
	}
	
	return true;
}

void UCharacterEquipmentComponent::GrabThrowableItem()
{
	if (!bThrowing)
	{
		return;
	}
	
	AGCProjectile* CurrentProjectile = ActiveThrowable->SpawnProjectile();
	CurrentProjectile->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,
		ActiveThrowable->GetCharacterEquippedSocketName());

	int32& ThrowablesCount = Pouch[(uint32)ActiveThrowable->GetAmmunitionType()];
	ThrowablesCountChanged.ExecuteIfBound(--ThrowablesCount);
}

void UCharacterEquipmentComponent::InterruptThrowingItem()
{
	CharacterOwner->OnActionEnded(ECharacterAction::ThrowItem);
	if (!bThrowing || !IsValid(ActiveThrowable))
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
	AEquippableItem* EquippedItem = GetEquippedItem();
	if (IsValid(EquippedItem))
	{
		EquippedItem->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform,
			EquippedItem->GetCharacterEquippedSocketName());
	}
}

void UCharacterEquipmentComponent::ActivateThrowableItem() const
{
	if (IsValid(ActiveThrowable))
	{
		ActiveThrowable->Activate(CharacterOwner->GetController());
	}
}

void UCharacterEquipmentComponent::ReleaseThrowableItem()
{
	if (!bThrowing || !IsValid(ActiveThrowable))
	{
		CharacterOwner->OnActionEnded(ECharacterAction::ThrowItem);
		return;
	}

	ActiveThrowable->Throw(CharacterOwner->GetController());
	bThrowing = false;
	AEquippableItem* EquippedItem = GetEquippedItem();
	if (IsValid(EquippedItem))
	{
		EquippedItem->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, EquippedItem->GetCharacterEquippedSocketName());
	}

	CharacterOwner->OnActionEnded(ECharacterAction::ThrowItem);
}

#pragma endregion THROWING

#pragma region MELEE

void UCharacterEquipmentComponent::StartPrimaryMeleeAttack(AController* AttackerController)
{
	StartMeleeAttack(EMeleeAttackType::Primary, AttackerController);
}

void UCharacterEquipmentComponent::StopPrimaryMeleeAttack()
{
	StopMeleeAttack(EMeleeAttackType::Primary);
}

void UCharacterEquipmentComponent::StartHeavyMeleeAttack(AController* AttackerController)
{
	StartMeleeAttack(EMeleeAttackType::Secondary, AttackerController);
}

void UCharacterEquipmentComponent::StopHeavyMeleeAttack()
{
	StopMeleeAttack(EMeleeAttackType::Secondary);
}

void UCharacterEquipmentComponent::StartMeleeAttack(EMeleeAttackType AttackType, AController* AttackerController)
{
	if (!IsValid(EquippedMeleeWeapon) || bMeleeAttack || !CharacterOwner->CanStartAction(ECharacterAction::MeleeAttack))
	{
		return;
	}

	const FMeleeAttackData* AttackData = EquippedMeleeWeapon->GetMeleeAttackData(AttackType);
	if (!IsValid(AttackData->Montage) || CharacterOwner->CharacterAttributesComponent->GetStamina() < AttackData->StaminaConsumption)
	{
		return;
	}
	
	bMeleeAttack = true;
	CharacterOwner->OnActionStarted(ECharacterAction::MeleeAttack);
	EquippedMeleeWeapon->StartAttack(AttackType, AttackerController);
	CharacterOwner->CharacterAttributesComponent->ChangeStaminaValue(-AttackData->StaminaConsumption);
	float Duration = AttackData->DesiredDuration * MeleeDurationMultiplier;
	if (AttackData->Montage->HasRootMotion())
	{
		CharacterOwner->bUseControllerRotationYaw = true;	
	}
	
	CharacterOwner->PlayAnimMontageWithDuration(AttackData->Montage, Duration);
	GetWorld()->GetTimerManager().SetTimer(MeleeAttackTimer, this, &UCharacterEquipmentComponent::OnMeleeAttackCompleted, Duration);
}

void UCharacterEquipmentComponent::StopMeleeAttack(EMeleeAttackType AttackType)
{
	if (!IsValid(EquippedMeleeWeapon) || !bMeleeAttack)
	{
		CharacterOwner->OnActionEnded(ECharacterAction::MeleeAttack);
		return;
	}

	const FMeleeAttackData* AttackData = EquippedMeleeWeapon->GetMeleeAttackData(AttackType);
	if (!AttackData || !AttackData->bChargeable)
	{
		return;
	}
	
	CharacterOwner->StopAnimMontage(AttackData->Montage);
	GetWorld()->GetTimerManager().ClearTimer(MeleeAttackTimer);
	OnMeleeAttackCompleted();
}

void UCharacterEquipmentComponent::OnMeleeAttackCompleted()
{
	bMeleeAttack = false;
	CharacterOwner->OnActionEnded(ECharacterAction::MeleeAttack);
	if (!IsValid(EquippedMeleeWeapon))
	{
		return;
	}
	
	const FMeleeAttackData* ActiveAttack = EquippedMeleeWeapon->GetActiveAttack();
	if (ActiveAttack && IsValid(ActiveAttack->Montage) && ActiveAttack->Montage->HasRootMotion())
	{
		CharacterOwner->UpdateStrafingControls();
	}
	
	if (IsValid(EquippedMeleeWeapon))
	{
		EquippedMeleeWeapon->StopAttack();
	}
}

void UCharacterEquipmentComponent::SetMeleeHitRegEnabled(bool bEnabled)
{
	if (IsValid(EquippedMeleeWeapon) && bMeleeAttack)
	{
		EquippedMeleeWeapon->SetIsHitRegistrationEnabled(bEnabled);
	}
}

#pragma endregion MELEE

EEquippableItemType UCharacterEquipmentComponent::GetEquippedItemType() const
{
	auto EquippedItem = GetEquippedItem();
	return IsValid(EquippedItem) ? EquippedItem->GetEquippableItemType() : EEquippableItemType::None;
}

void UCharacterEquipmentComponent::OnAmmoChanged(int32 ClipAmmo) const
{
	if (IsValid(EquippedRangedWeapon))
	{
		WeaponAmmoChangedEvent.ExecuteIfBound(ClipAmmo, Pouch[(uint32)EquippedRangedWeapon->GetAmmunitionType()]);
	}
}

AEquippableItem* UCharacterEquipmentComponent::GetEquippedItem() const
{
	if (IsValid(EquippedRangedWeapon))
	{
		return EquippedRangedWeapon;
	}
	else if (IsValid(EquippedMeleeWeapon))
	{
		return EquippedMeleeWeapon;
	}

	return nullptr;
}

bool UCharacterEquipmentComponent::IsPreferStrafing() const
{
	AEquippableItem* EquippabeItem = GetEquippedItem();
	return IsValid(EquippabeItem) ? EquippabeItem->IsRangedWeapon() : false;
}
