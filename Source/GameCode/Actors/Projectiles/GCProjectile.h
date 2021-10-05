// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GCProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

DECLARE_DELEGATE_TwoParams(FProjectileHitEvent, const FHitResult& HitResult, const FVector& ShotDirection);

UCLASS()
class GAMECODE_API AGCProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AGCProjectile();

	mutable FProjectileHitEvent ProjectileHitEvent;
	
	void LaunchProjectile(FVector Direction, float Speed, AController* ThrowerController);
	
	virtual void Activate(AController* ThrowerController) { CachedThrowerController = ThrowerController; }
	void Drop(AController* ThrowerController);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	    FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USphereComponent* CollisionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UProjectileMovementComponent* ProjectileMovementComponent;

	virtual void OnProjectileLaunched() {}

	TWeakObjectPtr<AController> CachedThrowerController;
};
