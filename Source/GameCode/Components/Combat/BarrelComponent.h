#pragma once

#include "CoreMinimal.h"
#include "GCDebugSubsystem.h"
#include "Components/SceneComponent.h"
#include "Data/DecalSettings.h"
#include "BarrelComponent.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class EHitRegistrationType : uint8
{
	HitScan,
	Projectile
};

UCLASS( ClassGroup=(Custom), Abstract )
class GAMECODE_API UBarrelComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	virtual void Shoot(const FVector& ViewLocation, const FVector& Direction, AController* ShooterController);
	virtual void ApplyDamage(const FHitResult& ShotResult, const FVector& Direction, AController* ShooterController) const;
	virtual void FinalizeShot() const;

	int32 GetAmmo() const { return Ammo; }
	void SetAmmo(int32 NewValue) { Ammo = NewValue; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Range = 5000.f;

	// per bullet or projectile. If Amount of bullets per shot > 1 (shotgun for example) then this damage is applied for each hitscan/projectile
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float InitialDamage = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* TraceFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDecalSettings DecalSettings; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EHitRegistrationType HitRegistrationType = EHitRegistrationType::HitScan;
	
	// MUst be normalized 0..1 on both axis. Curve value will be multiplied by InitialDamage and applied to actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveFloat* DamageFalloffDiagram;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "HitRegistrationType == EHitRegistrationType::Projectile"))
	TSubclassOf<class AGCProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=200.f, UIMin = 200.f, EditCondition = "HitRegistrationType == EHitRegistrationType::Projectile"))
	float ProjectileSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USoundCue* ShotSound;

	virtual AActor* GetDamagingActor() const { return GetOwner(); }
	
private:

	bool ShootHitScan(const FVector& ViewLocation, const FVector& Direction, AController* ShooterController);
	bool ShootProjectile(const FVector& ViewLocation, const FVector& ViewDirection, AController* ShooterController);
	void SpawnBulletHole(const FHitResult& HitResult);
	void OnProjectileHit(const FHitResult& HitResult, const FVector& Direction);
	TWeakObjectPtr<AController> CachedShooterController = nullptr;

	int32 Ammo = 0;
	
	#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	const UGCDebugSubsystem* GetDebugSubsystem() const;
	mutable UGCDebugSubsystem* DebugSubsystem;
	#endif
};
