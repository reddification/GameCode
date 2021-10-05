// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/ExplosiveProjectile.h"

#include "Components/ExplosionComponent.h"

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

void AExplosiveProjectile::Detonate()
{
	if (CachedThrowerController.IsValid())
	{
		ExplosionComponent->Explode(CachedThrowerController.Get());
	}
}
