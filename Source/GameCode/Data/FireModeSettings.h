#pragma once
#include "EquipmentTypes.h"
#include "UserInterfaceTypes.h"
#include "FireModeSettings.generated.h"

USTRUCT(BlueprintType)
struct FFireModeSettings
{
	GENERATED_BODY()
	// shots per minutes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 1.f, UIMin = 1.f))
	float FireRate = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EWeaponFireMode FireMode = EWeaponFireMode::FullAuto;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Weapons")
	UAnimMontage* WeaponShootMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Weapons")
	UAnimMontage* WeaponReloadMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Character")
	UAnimMontage* CharacterShootMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Character")
	UAnimMontage* CharacterReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animations|Character")
	UAnimMontage* SwitchMontage;
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 0.01f, UIMin = 0.01f))
	float SwitchDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USoundCue* ChangeSFX;

	// bullet spread half angle in degrees
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin = 1.f, UIMin = 1.f, ClampMax = 10.f, UIMax = 10.f))
	float SpreadAngle = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bCanAim = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim", meta=(EditCondition = "bCanAim == true", ClampMin = 0.f, UIMin = 0.f, ClampMax = 5.f, UIMax = 5.f))
	float AimSpreadAngle = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim", meta=(EditCondition = "bCanAim == true", ClampMin = 1.f, UIMin = 1.f))
	float AimFOV = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim", meta=(EditCondition = "bCanAim == true", ClampMin = 0.f, UIMin = 0.f))
	float AimMaxSpeed = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim", meta=(EditCondition = "bCanAim == true", ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimTurnModifier = 0.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim", meta=(EditCondition = "bCanAim == true", ClampMin = 0.f, UIMin = 0.f, ClampMax = 1.f, UIMax = 1.f))
	float AimLookUpModifier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim", meta=(EditCondition = "bCanAim == true"))
	EReticleType AimReticleType = EReticleType::Crosshair;
};
