#pragma once

#include "CoreMinimal.h"
#include "Actors/CommonDelegates.h"
#include "Actors/Equipment/EquipableItem.h"
#include "Characters/GCBaseCharacter.h"
#include "Data/EquipmentTypes.h"
#include "RangeWeaponItem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FShootEvent, class UAnimMontage* CharacterShootMontage)
DECLARE_DELEGATE(FOutOfAmmoEvent)

class UAnimMontage;

UENUM()
enum class EWeaponFireMode
{
	Single,
	Burst,
	FullAuto,
};

UCLASS(Blueprintable)
class GAMECODE_API ARangeWeaponItem : public AEquipableItem
{
	GENERATED_BODY()
	
public:
	ARangeWeaponItem();

	bool TryStartFiring(AController* ShooterController);
	void StopFiring();

	void StartAiming();
	void StopAiming();
	void StartReloading(const float DesiredReloadDuration);
	void StopReloading(bool bInterrupted);
	
	UAnimMontage* GetCharacterShootMontage() const { return CharacterShootMontage; }
	UAnimMontage* GetCharacterReloadMontage() const { return CharacterReloadMontage; }
	const UAnimMontage* GetWeaponReloadMontage() const { return WeaponReloadMontage; }

	FTransform GetForegripTransform() const;
	EReloadType GetReloadType() const { return ReloadType; }
	USkeletalMeshComponent* GetMesh() { return WeaponMeshComponent; }
	EReticleType GetAimReticleType() const {return AimReticleType; }

	mutable FShootEvent ShootEvent;
	mutable FAmmoChangedEvent AmmoChangedEvent;
	mutable FOutOfAmmoEvent OutOfAmmoEvent;
	
	float GetAimFOV() const { return AimFOV; }
	float GetAimMaxSpeed() const { return AimMaxSpeed; }
	float GetAimTurnModifier() const { return AimTurnModifier; }
	float GetAimLookUpModifier() const { return AimLookUpModifier; }

	int32 GetAmmo() const { return Ammo; }
	void SetAmmo(int32 NewAmmo);
	int32 GetClipCapacity() const { return ClipCapacity; }
	EAmmunitionType GetAmmunitionType() const { return AmmunitionType; }

	float GetReloadDuration() const { return ReloadDuration; }

	virtual EReticleType GetReticleType() const override;

	const class UCameraComponent* GetScopeCameraComponent() const { return ScopeCameraComponent; }
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USkeletalMeshComponent* WeaponMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UWeaponBarrelComponent* WeaponBarrelComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UCameraComponent* ScopeCameraComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MuzzleSocketName = "muzzle_socket";

	// shots per minutes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 1.f, UIMin = 1.f))
	float FireRate = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EWeaponFireMode FireMode = EWeaponFireMode::FullAuto;

	// bullet spread half angle in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 1.f, UIMin = 1.f, ClampMax = 10.f, UIMax = 10.f))
	float SpreadAngle = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.f, UIMin = 0.f, ClampMax = 5.f, UIMax = 5.f))
	float AimSpreadAngle = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 1.f, UIMin = 1.f))
	float AimFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.f, UIMin = 0.f))
	float AimMaxSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimTurnModifier = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimLookUpModifier = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Weapons")
	UAnimMontage* WeaponShootMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Weapons")
	UAnimMontage* WeaponReloadMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Character")
	UAnimMontage* CharacterShootMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Character")
	UAnimMontage* CharacterReloadMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAmmunitionType AmmunitionType = EAmmunitionType::None;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 1.f, UIMin = 1.f))
	int32 ClipCapacity = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.01f, UIMin = 0.01f))
	float ReloadDuration = 1.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin=1, UIMin=1))
	int32 BulletsPerShot = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReloadType ReloadType = EReloadType::FullClip;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReticleType AimReticleType = EReticleType::Crosshair;
	
private:
	bool bAiming = false;
	bool bReloading = false;
	bool bFiring = false;
	int32 Ammo = 0;
	FTimerHandle ShootTimer;
	
	class AController* CachedShooterController = nullptr;

	float PlayAnimMontage(UAnimMontage* AnimMontage, float DesiredDuration = -1);
	float GetShootTimerInterval() const { return 60.f / FireRate; };
	void Shoot();
	void ResetShot();
	FVector GetBulletSpreadOffset(const FRotator& ShotOrientation) const;
	float GetBulletSpreadAngleRad () const;
};
