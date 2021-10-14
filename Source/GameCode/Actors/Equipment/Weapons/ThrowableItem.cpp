// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Equipment/Weapons/ThrowableItem.h"

#include "DrawDebugHelpers.h"
#include "Actors/Projectiles/GCProjectile.h"

void AThrowableItem::BeginPlay()
{
	Super::BeginPlay();
	AttachedProjectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass, GetActorLocation(), FRotator::ZeroRotator);
	AttachedProjectile->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	AttachedProjectile->SetOwner(GetOwner());
}

void AThrowableItem::Throw(AController* OwnerController)
{
	FVector ViewPoint;
	FRotator ViewRotation;
	OwnerController->GetPlayerViewPoint(ViewPoint, ViewRotation);
	FVector LaunchDirection = ViewRotation.Vector();

	CurrentProjectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	FHitResult TraceResult;
	const FVector TraceEnd = ViewPoint + LaunchDirection * ThrowSpeed;
	bool bHit = GetWorld()->LineTraceSingleByChannel(TraceResult, ViewPoint, TraceEnd, ECC_Visibility);
	LaunchDirection = bHit || TraceResult.bBlockingHit
		? (TraceResult.ImpactPoint - CurrentProjectile->GetActorLocation()).GetSafeNormal()
		: (TraceEnd - CurrentProjectile->GetActorLocation()).GetSafeNormal();

	FVector ViewUpVector = ViewRotation.RotateVector(FVector::UpVector);
	LaunchDirection = LaunchDirection + FMath::Tan(FMath::DegreesToRadians(ThrowAngle)) * ViewUpVector;
	
	CurrentProjectile->LaunchProjectile(LaunchDirection.GetSafeNormal(), GetOwner()->GetVelocity().Size() + ThrowSpeed, OwnerController);
}

void AThrowableItem::Activate(AController* Controller)
{
	if (CurrentProjectile.IsValid())
	{
		CurrentProjectile->Activate(Controller);
	}
}

AGCProjectile* AThrowableItem::GetCurrentProjectile()
{
	return CurrentProjectile.Get(); 
}

AGCProjectile* AThrowableItem::SpawnProjectile()
{
	CurrentProjectile = GetWorld()->SpawnActor<AGCProjectile>(ProjectileClass);
	CurrentProjectile->SetOwner(GetOwner());
	return CurrentProjectile.Get();
}

void AThrowableItem::DropProjectile(AController* Controller)
{
	if (CurrentProjectile.IsValid())
	{
		CurrentProjectile->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		CurrentProjectile->Drop(Controller);
	}
}
