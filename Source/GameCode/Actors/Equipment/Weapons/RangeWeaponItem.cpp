// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/RangeWeaponItem.h"

#include <PxMathUtils.h>

#include "GameCode.h"
#include "Components/WeaponBarrelComponent.h"

ARangeWeaponItem::ARangeWeaponItem()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMeshComponent->SetupAttachment(RootComponent);

	WeaponBarrelComponent = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("Barrel"));
	WeaponBarrelComponent->SetupAttachment(WeaponMeshComponent, MuzzleSocketName);
}

bool ARangeWeaponItem::TryStartFiring(AController* ShooterController)
{
	CachedShooterController = ShooterController;
	switch (FireMode)
	{
		case EWeaponFireMode::Single:
			Shoot();
		case EWeaponFireMode::Burst:
			// TODO
			break;
		case EWeaponFireMode::FullAuto:
			Shoot();
			GetWorld()->GetTimerManager().ClearTimer(AutoShootTimer);
			GetWorld()->GetTimerManager().SetTimer(AutoShootTimer, this, &ARangeWeaponItem::Shoot, GetShootTimerInterval(), true);
			break;
		default:
			break;
	}
	
	return true;
}

void ARangeWeaponItem::StopFiring()
{
	if (FireMode == EWeaponFireMode::FullAuto)
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoShootTimer);
	}
}

void ARangeWeaponItem::StartAiming()
{
	bAiming = true;
}

void ARangeWeaponItem::StopAiming()
{
	bAiming = false;
}

void ARangeWeaponItem::StartReloading(float DesiredReloadDuration)
{
	bReloading = true;
	PlayAnimMontage(WeaponReloadMontage, DesiredReloadDuration);
}

void ARangeWeaponItem::StopReloading(bool bInterrupted)
{
	bReloading = false;
	if (bInterrupted)
	{
		WeaponMeshComponent->GetAnimInstance()->Montage_Stop(0.0f, WeaponReloadMontage);
	}
}

void ARangeWeaponItem::Shoot()
{
	if (Ammo <= 0)
	{
		if (!bReloading)
		{
			OutOfAmmoEvent.ExecuteIfBound();
			StopFiring();
		}
		return;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	CachedShooterController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FVector ViewDirection = ViewRotation.Vector();
	ViewDirection += GetBulletSpreadOffset(ViewRotation);
	WeaponBarrelComponent->Shoot(ViewLocation, ViewDirection, CachedShooterController);
	SetAmmo(Ammo - 1);
	PlayAnimMontage(WeaponShootMontage);
	if (ShootEvent.IsBound())
	{
		ShootEvent.Broadcast(CharacterShootMontage);
	}
}

FVector ARangeWeaponItem::GetBulletSpreadOffset(const FRotator& ShotOrientation) const
{
	float PitchAngle = FMath::RandRange(0.f, GetBulletSpreadAngleRad());
	float SpreadSize = FMath::Tan(PitchAngle);
	float RotationAngle = FMath::RandRange(0.f, 2 * PI);
	float SpreadY = FMath::Cos(RotationAngle);
	float SpreadZ = FMath::Sin(RotationAngle);

	return (ShotOrientation.RotateVector(FVector::UpVector) * SpreadZ + ShotOrientation.RotateVector(FVector::RightVector) * SpreadY) * SpreadSize;
}

float ARangeWeaponItem::GetBulletSpreadAngleRad() const
{
	return FMath::DegreesToRadians(bAiming ? AimSpreadAngle : SpreadAngle); 
}

void ARangeWeaponItem::SetAmmo(int32 NewAmmo)
{
	Ammo = NewAmmo;
	AmmoChangedEvent.ExecuteIfBound(NewAmmo);
}

FTransform ARangeWeaponItem::GetForegripTransform() const
{
	return WeaponMeshComponent->GetSocketTransform(SocketAssaultRifleForegrip);
}

void ARangeWeaponItem::BeginPlay()
{
	Super::BeginPlay();
	SetAmmo(ClipCapacity);
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
