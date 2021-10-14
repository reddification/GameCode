#pragma once

#include "CoreMinimal.h"
#include "Actors/CommonDelegates.h"
#include "Actors/Equipment/EquippableItem.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/WeaponBarrelComponent.h"
#include "Data/EquipmentTypes.h"
#include "RangeWeaponItem.generated.h"

class UAnimMontage;

DECLARE_MULTICAST_DELEGATE_OneParam(FShootEvent, UAnimMontage* CharacterShootMontage)
DECLARE_DELEGATE(FOutOfAmmoEvent)

class UAnimMontage;

UCLASS(Blueprintable)
class GAMECODE_API ARangeWeaponItem : public AEquippableItem
{
	GENERATED_BODY()
	
public:
	ARangeWeaponItem();

	bool TryStartFiring(AController* ShooterController);
	void StopFiring();

	bool StartAiming();
	void StopAiming();
	void StartReloading(const float DesiredReloadDuration);
	void StopReloading(bool bInterrupted);
	
	UAnimMontage* GetCharacterShootMontage() const { return ActiveWeaponBarrel->GetFireModeSettings().CharacterShootMontage; }
	UAnimMontage* GetCharacterReloadMontage() const { return ActiveWeaponBarrel->GetFireModeSettings().CharacterReloadMontage; }
	const UAnimMontage* GetWeaponReloadMontage() const { return ActiveWeaponBarrel->GetFireModeSettings().WeaponReloadMontage; }

	FTransform GetForegripTransform() const;
	EReloadType GetReloadType() const { return ActiveWeaponBarrel->GetFireModeSettings().ReloadType; }
	USkeletalMeshComponent* GetMesh() { return WeaponMeshComponent; }
	EReticleType GetAimReticleType() const {return ActiveWeaponBarrel->GetFireModeSettings().AimReticleType; }

	void StartTogglingFireMode();
	bool CanToggleFireMode() const { return Barrels.Num() > 1 && !(bFiring || bChangingFireMode); }
	void CompleteTogglingFireMode();
	const FFireModeSettings& GetNextFireModeSettings () const;
	bool CanReload() const { return !bChangingFireMode; }

	mutable FShootEvent ShootEvent;
	mutable FAmmoChangedEvent AmmoChangedEvent;
	mutable FOutOfAmmoEvent OutOfAmmoEvent;
	
	float GetAimFOV() const { return ActiveWeaponBarrel->GetFireModeSettings().AimFOV; }
	float GetAimMaxSpeed() const { return ActiveWeaponBarrel->GetFireModeSettings().AimMaxSpeed; }
	float GetAimTurnModifier() const { return ActiveWeaponBarrel->GetFireModeSettings().AimTurnModifier; }
	float GetAimLookUpModifier() const { return ActiveWeaponBarrel->GetFireModeSettings().AimLookUpModifier; }

	int32 GetAmmo() const { return ActiveWeaponBarrel->GetAmmo(); }
	void SetAmmo(int32 NewAmmo);
	int32 GetClipCapacity() const { return ActiveWeaponBarrel->GetFireModeSettings().ClipCapacity; }
	EAmmunitionType GetAmmunitionType() const { return ActiveWeaponBarrel->GetFireModeSettings().AmmunitionType; }

	float GetReloadDuration() const { return ActiveWeaponBarrel->GetFireModeSettings().ReloadDuration; }

	virtual EReticleType GetReticleType() const override;

	const class UCameraComponent* GetScopeCameraComponent() const { return ScopeCameraComponent; }

	bool CanAim() const { return ActiveWeaponBarrel->GetFireModeSettings().bCanAim; }
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USkeletalMeshComponent* WeaponMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UWeaponBarrelComponent* PrimaryWeaponBarrelComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UCameraComponent* ScopeCameraComponent;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName MuzzleSocketName = "muzzle_socket";
	
private:
	FTimerHandle ShootTimer;
	FTimerHandle ChangeFireModeTimer;
	
	bool bAiming = false;
	bool bReloading = false;
	bool bFiring = false;
	bool bChangingFireMode = false;
	
	class AController* CachedShooterController = nullptr;

	float PlayAnimMontage(UAnimMontage* AnimMontage, float DesiredDuration = -1);
	float GetShootTimerInterval() const { return 60.f / ActiveWeaponBarrel->GetFireModeSettings().FireRate; };
	bool Shoot();
	void ResetShot();
	FVector GetBulletSpreadOffset(const FRotator& ShotOrientation) const;
	float GetBulletSpreadAngleRad () const;

	// Fire modes to cycle through
	TArray<UWeaponBarrelComponent*> Barrels;
	TWeakObjectPtr<UWeaponBarrelComponent> ActiveWeaponBarrel;
	int ActiveBarrelIndex = 0;
};
