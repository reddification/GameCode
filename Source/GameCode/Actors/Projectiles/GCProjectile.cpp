// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/GCProjectile.h"

#include "GameCode.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

AGCProjectile::AGCProjectile()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->InitialSpeed = 2000.f;
	ProjectileMovementComponent->bSimulationEnabled = false;
	AddOwnedComponent(ProjectileMovementComponent);

	RotatingMovementComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement"));
	AddOwnedComponent(RotatingMovementComponent);
}

void AGCProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (bDestroyOnHit)
	{
		CollisionComponent->OnComponentHit.AddDynamic(this, &AGCProjectile::DestroyOnHit);
	}
	
	if (bRotate)
	{
		ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AGCProjectile::OnProjectileStopped);
		ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AGCProjectile::OnProjectileBounced);
	}
}

void AGCProjectile::LaunchProjectile(FVector Direction, float Speed, AController* ThrowerController)
{
	ProjectileMovementComponent->Velocity = Direction * Speed;
	ProjectileMovementComponent->bSimulationEnabled = true;
	CollisionComponent->SetCollisionProfileName(ProfileProjectile);
	AActor* ProjectileOwner = GetOwner();
	while (IsValid(ProjectileOwner))
	{
		CollisionComponent->IgnoreActorWhenMoving(ProjectileOwner, true);
		ProjectileOwner = ProjectileOwner->GetOwner();
	}
	
	this->CachedThrowerController = ThrowerController; 
	OnProjectileLaunched();

	if (bRotate)
	{
		RotatingMovementComponent->RotationRate = FRotator(BaseRotationRate.Pitch * Direction.Y,
			BaseRotationRate.Yaw * Direction.Z, BaseRotationRate.Roll * Direction.X) * ProjectileMovementComponent->Velocity.Size();
	}
	else
	{
		ProjectileMovementComponent->bRotationFollowsVelocity = 1;
	}
}

void AGCProjectile::Drop(AController* ThrowerController)
{
	CachedThrowerController = ThrowerController;
	ProjectileMovementComponent->Velocity = -FVector::UpVector * 5.f;
	ProjectileMovementComponent->bSimulationEnabled = true;
	CollisionComponent->SetCollisionProfileName(ProfileProjectile);
	CollisionComponent->IgnoreActorWhenMoving(GetOwner(), true);
}

void AGCProjectile::DestroyOnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ProjectileHitEvent.ExecuteIfBound(Hit, ProjectileMovementComponent->Velocity.GetSafeNormal());
	Destroy();
	// TODO expose UStaticMeshComponent and use SetLifeSpan instead of immediately destroying?
	// SetLifeSpan(2.f); 	
}

void AGCProjectile::OnProjectileStopped(const FHitResult& ImpactResult)
{
}


void AGCProjectile::OnProjectileBounced(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if (bRotate)
	{
		RotatingMovementComponent->RotationRate = FRotator(BaseRotationRate.Pitch * ImpactResult.ImpactNormal.Y,
		BaseRotationRate.Yaw * ImpactResult.ImpactNormal.Z, BaseRotationRate.Roll * ImpactResult.ImpactNormal.X)
			* ImpactVelocity.Size();
	}
}