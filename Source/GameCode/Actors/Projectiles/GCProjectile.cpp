// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/GCProjectile.h"

#include "GameCode.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AGCProjectile::AGCProjectile()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->bSimulationEnabled = false;
	AddOwnedComponent(ProjectileMovementComponent);
}

void AGCProjectile::LaunchProjectile(FVector Direction, float Speed, AController* ThrowerController)
{
	ProjectileMovementComponent->Velocity = Direction * Speed;
	ProjectileMovementComponent->bSimulationEnabled = true;
	CollisionComponent->SetCollisionProfileName(ProfileProjectile);
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
	this->CachedThrowerController = ThrowerController; 
	OnProjectileLaunched();
}

void AGCProjectile::Drop(AController* ThrowerController)
{
	CachedThrowerController = ThrowerController;
	ProjectileMovementComponent->Velocity = -FVector::UpVector * 5.f;
	ProjectileMovementComponent->bSimulationEnabled = true;
	CollisionComponent->SetCollisionProfileName(ProfileProjectile);
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);

}

void AGCProjectile::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentHit.AddDynamic(this, &AGCProjectile::OnCollisionHit);
}

void AGCProjectile::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ProjectileHitEvent.ExecuteIfBound(Hit, ProjectileMovementComponent->Velocity.GetSafeNormal());
	Destroy();
	// TODO expose UStaticMeshComponent and use SetLifeSpan instead of immediately destroying?
	// SetLifeSpan(2.f); 
}
