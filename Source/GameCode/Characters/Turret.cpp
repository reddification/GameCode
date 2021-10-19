// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Turret.h"

#include "GenericTeamAgentInterface.h"
#include "Components/Combat/TurretBarrelComponent.h"

ATurret::ATurret()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComponent);

	TurretBaseComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretBase"));
	TurretBaseComponent->SetupAttachment(RootComponent);

	TurretGunComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TurretGun"));
	TurretGunComponent->SetupAttachment(TurretBaseComponent);

	TurretBarrelComponent = CreateDefaultSubobject<UTurretBarrelComponent>(TEXT("WeaponBarrel"));
	TurretBarrelComponent->SetupAttachment(TurretGunComponent);
}

void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentMode)
	{
		case ETurretMode::Search:
			Search(DeltaTime);
		break;
		case ETurretMode::Attack:
			Track(DeltaTime);
		break;
		default:
			break;
	}
}

void ATurret::SetCurrentTarget(AActor* NewTarget)
{
	Target = NewTarget;
	KillableTarget = Cast<IKillable>(NewTarget);
	SetMode(Target.IsValid() ? ETurretMode::Attack : ETurretMode::Search);
}

FVector ATurret::GetPawnViewLocation() const
{
	return TurretBarrelComponent->GetComponentLocation();
}

FRotator ATurret::GetViewRotation() const
{
	return TurretBarrelComponent->GetComponentRotation();
}

void ATurret::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	IGenericTeamAgentInterface* AIController = Cast<IGenericTeamAgentInterface>(NewController);
	if (!AIController)
	{
		return;
	}

	AIController->SetGenericTeamId(FGenericTeamId((uint8)Team));
}

void ATurret::Search(float DeltaTime)
{
	FRotator TurretBaseRotation = TurretBaseComponent->GetRelativeRotation();
	TurretBaseRotation.Yaw += DeltaTime * BaseSearchRotationRate;
	TurretBaseComponent->SetRelativeRotation(TurretBaseRotation);

	FRotator TurretGunRotation = TurretGunComponent->GetRelativeRotation();
	TurretGunRotation.Pitch = FMath::FInterpTo(TurretGunRotation.Pitch, 0.f, DeltaTime, BarrelPitchRotationRate);
	TurretGunComponent->SetRelativeRotation(TurretGunRotation);
}

void ATurret::Track(float DeltaTime)
{
	FVector BaseLookAtDirection = (Target->GetActorLocation() - TurretBaseComponent->GetComponentLocation()).GetSafeNormal2D();
	FQuat LookAtQuat = BaseLookAtDirection.ToOrientationQuat();
	FQuat TargetQuat = FMath::QInterpTo(TurretBaseComponent->GetComponentQuat(), LookAtQuat, DeltaTime, LookAtInterpSpeed);
	TurretBaseComponent->SetWorldRotation(TargetQuat);

	FVector GunLookAtDirection = (Target->GetActorLocation() - TurretBarrelComponent->GetComponentLocation()).GetSafeNormal();
	float GunLookAtPitch = GunLookAtDirection.ToOrientationRotator().Pitch;
	FRotator CurrentGunRotation = TurretGunComponent->GetRelativeRotation();
	CurrentGunRotation.Pitch = FMath::FInterpTo(CurrentGunRotation.Pitch, GunLookAtPitch, DeltaTime, LookAtInterpSpeed);
	TurretGunComponent->SetRelativeRotation(CurrentGunRotation);
}

void ATurret::SetMode(ETurretMode NewMode)
{
	if (NewMode == CurrentMode)
	{
		return;
	}
	
	CurrentMode = NewMode;
	switch (CurrentMode)
	{
		case ETurretMode::Search:
			GetWorld()->GetTimerManager().ClearTimer(ShootDelayTimer);
		break;
		case ETurretMode::Attack:
			GetWorld()->GetTimerManager().SetTimer(ShootDelayTimer, this, &ATurret::Shoot, GetFireInterval(), true, FireDelayTime);
		break;
	}	
}

void ATurret::Shoot()
{
	TurretBarrelComponent->Shoot(TurretBarrelComponent->GetComponentLocation(), TurretBarrelComponent->GetComponentRotation().Vector(), Controller);
	TurretBarrelComponent->FinalizeShot();
	if (KillableTarget && !KillableTarget->IsAlive())
	{
		SetCurrentTarget(nullptr);
	}
}

