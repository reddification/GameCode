#include "BarrelComponent.h"

#include "DrawDebugHelpers.h"
#include "GameCode.h"
#include "GCDebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Actors/Projectiles/GCProjectile.h"
#include "Components/DecalComponent.h"
#include "Sound/SoundCue.h"

void UBarrelComponent::Shoot(const FVector& ViewLocation, const FVector& Direction, AController* ShooterController)
{
	bool bHit = false;
	switch (HitRegistrationType)
	{
		case EHitRegistrationType::HitScan:
			bHit = ShootHitScan(ViewLocation, Direction, ShooterController);
			break;
		case EHitRegistrationType::Projectile:
			ShootProjectile(ViewLocation, Direction, ShooterController);
			break;
		default:
			break;
	}
}

bool UBarrelComponent::ShootHitScan(const FVector& ViewLocation, const FVector& Direction,
                                          AController* ShooterController)
{
#if ENABLE_DRAW_DEBUG
	bool bDrawDebugEnabled = GetDebugSubsystem()->IsDebugCategoryEnabled(DebugCategoryRangeWeapons);
#else
	bool bDrawDebugEnabled = false;
#endif

	FVector ProjectileStartLocation = GetComponentLocation();
	FVector ProjectileEndLocation = ViewLocation + Range * Direction;
	const UWorld* World = GetWorld();
	FHitResult ShotResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	CollisionQueryParams.AddIgnoredActor(GetOwner()->GetOwner());
	bool bHit = World->LineTraceSingleByChannel(ShotResult, ViewLocation, ProjectileEndLocation, ECC_Bullet, CollisionQueryParams);
	// TODO DotProduct doesnt really solve the problem of shooting behind players back. Need to fix one day
	if (bHit && FVector::DotProduct(Direction, ShotResult.ImpactPoint - ProjectileStartLocation) > 0.f)
	{
		ProjectileEndLocation = ShotResult.ImpactPoint + Direction * 5.f;
	}

	bHit = World->LineTraceSingleByChannel(ShotResult, ProjectileStartLocation, ProjectileEndLocation, ECC_Bullet, CollisionQueryParams);
	if (bHit)
	{
		ProjectileEndLocation = ShotResult.ImpactPoint;
		ApplyDamage(ShotResult, Direction, ShooterController);
		
		if (bDrawDebugEnabled)
		{
			DrawDebugSphere(World, ProjectileEndLocation, 10.f, 24, FColor::Red, false, 2.f);
		}

		SpawnBulletHole(ShotResult);
	}

	if (bDrawDebugEnabled)
	{
		DrawDebugLine(World, ProjectileStartLocation, ProjectileEndLocation, FColor::Red, false, 2.f);
	}

	UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TraceFX, ProjectileStartLocation, GetComponentRotation());
	TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ProjectileEndLocation);

	return bHit;
}

bool UBarrelComponent::ShootProjectile(const FVector& ViewLocation, const FVector& ViewDirection,
	AController* ShooterController)
{
	CachedShooterController = ShooterController;
	FVector ShootDirection = (ViewLocation + ViewDirection * Range) - GetComponentLocation();
	AGCProjectile* CurrentProjectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass, GetComponentLocation(), FRotator::ZeroRotator);
	CurrentProjectile->SetOwner(GetOwner());
	FHitResult TraceResult;
	const FVector TraceEnd = ViewLocation + ShootDirection * Range;
	bool bHit = GetWorld()->LineTraceSingleByChannel(TraceResult, ViewLocation, TraceEnd, ECC_Visibility);
	ShootDirection = bHit || TraceResult.bBlockingHit
		? (TraceResult.ImpactPoint - CurrentProjectile->GetActorLocation()).GetSafeNormal()
		: (TraceEnd - CurrentProjectile->GetActorLocation()).GetSafeNormal();
	CurrentProjectile->SetActorRotation(ShootDirection.ToOrientationRotator());
	CurrentProjectile->ProjectileHitEvent.BindUObject(this, &UBarrelComponent::OnProjectileHit);
	
	CurrentProjectile->LaunchProjectile(ShootDirection.GetSafeNormal(), GetOwner()->GetVelocity().Size() + ProjectileSpeed, ShooterController);
	return true;
}

void UBarrelComponent::OnProjectileHit(const FHitResult& HitResult, const FVector& Direction)
{
	if (CachedShooterController.IsValid())
	{
		ApplyDamage(HitResult, Direction, CachedShooterController.Get());
	}
	
	SpawnBulletHole(HitResult);
}

void UBarrelComponent::ApplyDamage(const FHitResult& ShotResult, const FVector& Direction, AController* ShooterController) const
{
	AActor* HitActor = ShotResult.GetActor();
	// Perhaps its better to use squared distance
	float Damage = IsValid(DamageFalloffDiagram)
		? DamageFalloffDiagram->GetFloatValue(ShotResult.Distance / Range) * InitialDamage
		: InitialDamage;
		
	if (IsValid(HitActor))
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.HitInfo = ShotResult;
		DamageEvent.ShotDirection = Direction;
		DamageEvent.DamageTypeClass = DamageTypeClass;
		HitActor->TakeDamage(Damage, DamageEvent, ShooterController, GetDamagingActor());	
	}
}

void UBarrelComponent::SpawnBulletHole(const FHitResult& HitResult)
{
	UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), DecalSettings.Material, DecalSettings.Size, HitResult.ImpactPoint,
																			HitResult.ImpactNormal.ToOrientationRotator());
	if (IsValid(DecalComponent))
	{
		DecalComponent->SetFadeOut(DecalSettings.LifeTime, DecalSettings.FadeOutTime);
		DecalComponent->SetFadeScreenSize(0.0001f);
	}
}

void UBarrelComponent::FinalizeShot() const
{
	if (IsValid(MuzzleFlashFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, GetComponentLocation(), GetComponentRotation());
	}
	
	if (IsValid(ShotSound))
	{
		UGameplayStatics::SpawnSoundAttached(ShotSound, GetAttachmentRoot());
	}
}

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

const UGCDebugSubsystem* UBarrelComponent::GetDebugSubsystem() const
{
	if (!IsValid(DebugSubsystem))
	{
		DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UGCDebugSubsystem>();
	}

	return DebugSubsystem;
}

#endif	

