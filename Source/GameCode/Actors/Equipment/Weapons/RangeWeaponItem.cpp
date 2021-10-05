// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/RangeWeaponItem.h"

#include "GameCode.h"
#include "Camera/CameraComponent.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/WeaponBarrelComponent.h"

ARangeWeaponItem::ARangeWeaponItem()
{
	WeaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMeshComponent->SetupAttachment(RootComponent);

	WeaponBarrelComponent = CreateDefaultSubobject<UWeaponBarrelComponent>(TEXT("Barrel"));
	WeaponBarrelComponent->SetupAttachment(WeaponMeshComponent, MuzzleSocketName);

	ScopeCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Scope"));
	ScopeCameraComponent->SetupAttachment(WeaponMeshComponent);
	
	ReticleType = EReticleType::Crosshair;
}

EReticleType ARangeWeaponItem::GetReticleType() const
{
	return bAiming ? AimReticleType : ReticleType;
}

void ARangeWeaponItem::BeginPlay()
{
	Super::BeginPlay();
	SetAmmo(ClipCapacity);
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
	Shoot();
	return true;
}

void ARangeWeaponItem::StopFiring()
{
	bFiring = false;
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
	for (auto i = 0; i < BulletsPerShot; i++)
	{
		WeaponBarrelComponent->Shoot(ViewLocation, ViewDirection + GetBulletSpreadOffset(ViewRotation), CachedShooterController);
	}

	SetAmmo(Ammo - 1);
	PlayAnimMontage(WeaponShootMontage);
	WeaponBarrelComponent->FinalizeShot();
	if (ShootEvent.IsBound())
	{
		ShootEvent.Broadcast(CharacterShootMontage);
	}

	GetWorld()->GetTimerManager().SetTimer(ShootTimer, this, &ARangeWeaponItem::ResetShot, GetShootTimerInterval(), false);
}

void ARangeWeaponItem::ResetShot()
{
	if (!bFiring)
	{
		return;
	}

	switch (FireMode)
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
	return FMath::DegreesToRadians(bAiming ? AimSpreadAngle : SpreadAngle); 
}


#pragma endregion SHOOT

#pragma region AIM

void ARangeWeaponItem::StartAiming()
{
	bAiming = true;
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

#pragma endregion RELOAD


void ARangeWeaponItem::SetAmmo(int32 NewAmmo)
{
	Ammo = NewAmmo;
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
