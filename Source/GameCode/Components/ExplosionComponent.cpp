// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ExplosionComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

void UExplosionComponent::Explode(AController* Controller)
{
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(GetOwner());
	// ECC_Visibility is kinda lame here
	UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), MaxDamage, MinDamage, GetComponentLocation(),
		InnerRadius, OuterRadius, DamageFalloff, DamageTypeClass, IgnoredActors, GetOwner(), Controller, ECC_Visibility);	
	
	if (IsValid(ExplosionVFX))
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetComponentLocation());
	}
	
	if (ExplosionEvent.IsBound())
	{
		ExplosionEvent.Broadcast();
	}
}
