#include "Actors/Projectiles/ExplosiveProjectile.h"

#include "Components/Combat/ExplosionComponent.h"
#include "Kismet/GameplayStatics.h"

AExplosiveProjectile::AExplosiveProjectile()
{
	ExplosionComponent = CreateDefaultSubobject<UExplosionComponent>(TEXT("Explosion"));
	ExplosionComponent->SetupAttachment(RootComponent);
}

void AExplosiveProjectile::Activate(AController* ThrowerController)
{
	Super::Activate(ThrowerController);
	GetWorld()->GetTimerManager().SetTimer(DetonationTimer, this, &AExplosiveProjectile::Detonate, DetonationTime);
}

void AExplosiveProjectile::OnProjectileLaunched()
{
	Super::OnProjectileLaunched();
}

void AExplosiveProjectile::DestroyOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Detonate();
	Super::DestroyOnHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AExplosiveProjectile::Detonate()
{
	if (CachedThrowerController.IsValid())
	{
		ExplosionComponent->Explode(CachedThrowerController.Get());
	}
}
