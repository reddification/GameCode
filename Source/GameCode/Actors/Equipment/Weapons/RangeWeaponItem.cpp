// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/RangeWeaponItem.h"

#include "GameCode.h"
#include "Camera/CameraComponent.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/Combat/WeaponBarrelComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ARangeWeaponItem::ARangeWeaponItem()
{
	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMeshComponent->SetupAttachment(RootComponent);

	PrimaryWeaponBarrelComponent = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("PrimaryBarrel"));
	PrimaryWeaponBarrelComponent->SetupAttachment(WeaponMeshComponent, MuzzleSocketName);

	ScopeCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Scope"));
	ScopeCameraComponent->SetupAttachment(WeaponMeshComponent);
	
	ReticleType = EReticleType::Crosshair;
}

EReticleType ARangeWeaponItem::GetReticleType() const
{
	return bAiming ? ActiveWeaponBarrel->GetFireModeSettings().AimReticleType : ReticleType;
}

void ARangeWeaponItem::BeginPlay()
{
	Super::BeginPlay();

	TInlineComponentArray<UWeaponBarrelComponent*> BarrelComponents;
	GetComponents<UWeaponBarrelComponent>(BarrelComponents);
	for (auto BarrelComponent : BarrelComponents)
	{
		UWeaponBarrelComponent* Barrel = StaticCast<UWeaponBarrelComponent*>(BarrelComponent);
		Barrel->SetAmmo(Barrel->GetFireModeSettings().ClipCapacity);
		Barrels.Add(Barrel);
	}
	
	ActiveBarrelIndex = 0;
	ActiveWeaponBarrel = Barrels[ActiveBarrelIndex];
	AmmoChangedEvent.ExecuteIfBound(ActiveWeaponBarrel->GetAmmo());
}

#pragma region SHOOT

bool ARangeWeaponItem::TryStartFiring(AController* ShooterController)
{
	// why not bFiring?
	if (GetWorld()->GetTimerManager().IsTimerActive(ShootTimer))
	{
		return false;
	}
	
	CachedShooterController = ShooterController;
	bFiring = true;
	return Shoot();
}

void ARangeWeaponItem::StopFiring()
{
	bFiring = false;
}

bool ARangeWeaponItem::Shoot()
{
	int32 Ammo = GetAmmo();
	if (Ammo <= 0)
	{
		if (!bReloading)
		{
			OutOfAmmoEvent.ExecuteIfBound();
			StopFiring();
		}
		
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	CachedShooterController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FVector ViewDirection = ViewRotation.Vector();
	const FFireModeSettings& FireModeSettings = ActiveWeaponBarrel->GetFireModeSettings();
	for (auto i = 0; i < FireModeSettings.BulletsPerShot; i++)
	{
		ActiveWeaponBarrel->Shoot(ViewLocation, ViewDirection + GetBulletSpreadOffset(ViewRotation), CachedShooterController);
	}

	SetAmmo(Ammo - 1);
	PlayAnimMontage(FireModeSettings.WeaponShootMontage);
	ActiveWeaponBarrel->FinalizeShot();
	if (ShootEvent.IsBound())
	{
		ShootEvent.Broadcast(FireModeSettings.CharacterShootMontage);
	}

	GetWorld()->GetTimerManager().SetTimer(ShootTimer, this, &ARangeWeaponItem::ResetShot, GetShootTimerInterval(), false);
	return true;
}

void ARangeWeaponItem::ResetShot()
{
	if (!bFiring)
	{
		return;
	}

	switch (ActiveWeaponBarrel->GetFireModeSettings().FireMode)
	{
		case EWeaponFireMode::Single:
			StopFiring();
			break;
		case EWeaponFireMode::FullAuto:
			Shoot();
			break;
		default:
			StopFiring();
			break;
	}
}

FVector ARangeWeaponItem::GetBulletSpreadOffset(const FRotator& ShotOrientation) const
{
	float PitchAngle = FMath::RandRange(0.f, GetBulletSpreadAngleRad());
	if (FMath::IsNearlyZero(PitchAngle, KINDA_SMALL_NUMBER))
	{
		return FVector::ZeroVector;
	}
	
	float SpreadSize = FMath::Tan(PitchAngle);
	float RotationAngle = FMath::RandRange(0.f, 2 * PI);
	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	return (ShotOrientation.RotateVector(FVector::UpVector) * SpreadZ + ShotOrientation.RotateVector(FVector::RightVector) * SpreadY) * SpreadSize;
}

float ARangeWeaponItem::GetBulletSpreadAngleRad() const
{
	const FFireModeSettings& FireModeSettings = ActiveWeaponBarrel->GetFireModeSettings();
	return FMath::DegreesToRadians(bAiming ? FireModeSettings.AimSpreadAngle : FireModeSettings.SpreadAngle); 
}

#pragma endregion SHOOT

#pragma region AIM

bool ARangeWeaponItem::StartAiming()
{
	if (CanAim() && !bAiming)
	{
		bAiming = true;
		return true;
	}

	return false;
}

void ARangeWeaponItem::StopAiming()
{
	bAiming = false;
}

#pragma endregion AIM

#pragma region RELOAD

void ARangeWeaponItem::StartReloading(float DesiredReloadDuration)
{
	bReloading = true;
	PlayAnimMontage(ActiveWeaponBarrel->GetFireModeSettings().WeaponReloadMontage, DesiredReloadDuration);
}

void ARangeWeaponItem::StopReloading(bool bInterrupted)
{
	bReloading = false;
	if (bInterrupted)
	{
		WeaponMeshComponent->GetAnimInstance()->Montage_Stop(0.0f, ActiveWeaponBarrel->GetFireModeSettings().WeaponReloadMontage);
	}
}

#pragma endregion RELOAD

#pragma region FIRE MODES

void ARangeWeaponItem::StartTogglingFireMode()
{
	bChangingFireMode = true;
}

void ARangeWeaponItem::CompleteTogglingFireMode()
{
	ActiveBarrelIndex = (ActiveBarrelIndex + 1) % Barrels.Num();
	ActiveWeaponBarrel = Barrels[ActiveBarrelIndex];
	AmmoChangedEvent.ExecuteIfBound(ActiveWeaponBarrel->GetAmmo());
	bChangingFireMode = false;
}

const FFireModeSettings& ARangeWeaponItem::GetNextFireModeSettings() const
{
	UWeaponBarrelComponent* NextBarrel = Barrels[(ActiveBarrelIndex + 1) % Barrels.Num()];
	return NextBarrel->GetFireModeSettings();
}

#pragma endregion FIRE MODES

void ARangeWeaponItem::SetAmmo(int32 NewAmmo)
{
	ActiveWeaponBarrel->SetAmmo(NewAmmo);
	AmmoChangedEvent.ExecuteIfBound(NewAmmo);
}

FTransform ARangeWeaponItem::GetForegripTransform() const
{
	return WeaponMeshComponent->GetSocketTransform(SocketForegrip);
}

float ARangeWeaponItem::PlayAnimMontage(UAnimMontage* AnimMontage, float DesiredDuration)
{
	const float PlayRate = DesiredDuration > 0 ? AnimMontage->GetPlayLength() / DesiredDuration : 1;
	const auto AnimInstance = WeaponMeshComponent->GetAnimInstance();
	if (IsValid(AnimInstance))
	{
		return AnimInstance->Montage_Play(AnimMontage, PlayRate);
	}

	return -1.f;
}
