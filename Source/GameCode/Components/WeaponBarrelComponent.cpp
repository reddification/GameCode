#include "Components/WeaponBarrelComponent.h"

#include "DrawDebugHelpers.h"
#include "GameCode.h"
#include "GCDebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"

void UWeaponBarrelComponent::Shoot(const FVector& ViewLocation, const FVector& Direction, AController* ShooterController)
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
	if (bHit && FVector::DotProduct(Direction, ShotResult.ImpactPoint - ProjectileStartLocation) > 0.f)
	{
		ProjectileEndLocation = ShotResult.ImpactPoint + Direction * 5.f;
	}

	bHit = World->LineTraceSingleByChannel(ShotResult, ProjectileStartLocation, ProjectileEndLocation, ECC_Bullet, CollisionQueryParams);
	if (bHit)
	{
		ProjectileEndLocation = ShotResult.ImpactPoint;
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
			HitActor->TakeDamage(Damage, DamageEvent, ShooterController, GetOwner());	
		}
		
		if (bDrawDebugEnabled)
		{
			DrawDebugSphere(World, ProjectileEndLocation, 10.f, 24, FColor::Red, false, 2.f);
		}

		UDecalComponent* DecalComponent = UGameplayStatics::SpawnDecalAtLocation(World, DecalSettings.Material, DecalSettings.Size, ProjectileEndLocation,
			ShotResult.ImpactNormal.ToOrientationRotator());
		if (IsValid(DecalComponent))
		{
			DecalComponent->SetFadeOut(DecalSettings.LifeTime, DecalSettings.FadeOutTime);
			DecalComponent->SetFadeScreenSize(0.0001f);
		}
	}

	if (bDrawDebugEnabled)
	{
		DrawDebugLine(World, ProjectileStartLocation, ProjectileEndLocation, FColor::Red, false, 2.f);
	}

	UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TraceFX, ProjectileStartLocation, GetComponentRotation());
	TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ProjectileEndLocation);
}

void UWeaponBarrelComponent::FinalizeShot() const
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, GetComponentLocation(), GetComponentRotation());
}

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

const UGCDebugSubsystem* UWeaponBarrelComponent::GetDebugSubsystem() const
{
	if (!IsValid(DebugSubsystem))
	{
		DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UGCDebugSubsystem>();
	}

	return DebugSubsystem;
}

#endif	

