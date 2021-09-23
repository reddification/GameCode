// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"

#include "DrawDebugHelpers.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Components/CapsuleComponent.h"
#include "Components/CharacterEquipmentComponent.h"
#include "Data/Movement/MantlingMovementParameters.h"
#include "Data/Movement/Posture.h"
#include "Data/Movement/StopClimbingMethod.h"
#include "GameCode/GameCode.h"
#include "GameCode/Actors/Interactive/InteractiveActor.h"
#include "GameCode/Actors/Interactive/Environment/Ladder.h"
#include "GameCode/Actors/Interactive/Environment/Zipline.h"
#include "GameCode/Components/CharacterAttributesComponent.h"
#include "GameCode/Components/InverseKinematicsComponent.h"
#include "GameCode/Components/LedgeDetectionComponent.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"
#include "GameCode/Data/MontagePlayResult.h"
#include "GameCode/Data/Movement/ZiplineParams.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/KismetSystemLibrary.h"

AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GCMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());
	InverseKinematicsComponent = CreateDefaultSubobject<UInverseKinematicsComponent>(TEXT("InverseKinematics"));
	AddOwnedComponent(InverseKinematicsComponent);
	InverseKinematicsComponent->SetScale(GetActorScale().Z);

	LedgeDetectionComponent = CreateDefaultSubobject<ULedgeDetectionComponent>(TEXT("LedgeDetection"));
	AddOwnedComponent(LedgeDetectionComponent);

	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributesComponent>(TEXT("Attributes"));
	AddOwnedComponent(CharacterAttributesComponent);

	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("Equipment"));
	AddOwnedComponent(CharacterEquipmentComponent);
	
	GCMovementComponent->bNotifyApex = true;

	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	GCMovementComponent->ClimbableTopReached.BindUObject(this, &AGCBaseCharacter::OnClimbableTopReached);
	GCMovementComponent->StoppedClimbing.BindUObject(this, &AGCBaseCharacter::OnStoppedClimbing);
	GCMovementComponent->CrouchedOrProned.BindUObject(this, &AGCBaseCharacter::OnStartCrouchOrProne);
	GCMovementComponent->WokeUp.BindUObject(this, &AGCBaseCharacter::OnEndCrouchOrProne);
	GCMovementComponent->SlidingStateChangedEvent.BindUObject(this, &AGCBaseCharacter::OnSlidingStateChangedEvent);
	GCMovementComponent->ZiplineObstacleHit.BindUObject(this, &AGCBaseCharacter::OnZiplineObstacleHit);
	GCMovementComponent->WallrunBeginEvent.AddUObject(this, &AGCBaseCharacter::OnWallrunBegin);
	GCMovementComponent->WallrunEndEvent.AddUObject(this, &AGCBaseCharacter::OnWallrunEnd);
	
	MontageEndedUnlockControlsEvent.BindUObject(this, &AGCBaseCharacter::OnMontageEndedUnlockControls);

	CharacterAttributesComponent->OutOfHealthEvent.AddUObject(this, &AGCBaseCharacter::OnOutOfHealth);
	CharacterAttributesComponent->OutOfStaminaEvent.AddUObject(this, &AGCBaseCharacter::OnOutOfStamina);

	CharacterEquipmentComponent->WeaponEquippedChangedEvent.BindUObject(this, &AGCBaseCharacter::OnWeaponEquippedChanged);
	CharacterEquipmentComponent->WeaponPickedUpEvent.BindUObject(this, &AGCBaseCharacter::OnWeaponPickedUpChanged);
	CharacterEquipmentComponent->ChangingEquippedItemStarted.BindUObject(this, &AGCBaseCharacter::OnChangingEquippedItemStarted);
	
	OnTakeAnyDamage.AddDynamic(CharacterAttributesComponent, &UCharacterAttributesComponent::OnTakeAnyDamage);
	
	CharacterEquipmentComponent->CreateLoadout();
	UpdateStrafingControls();
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TryChangeSprintState();
	const EPosture CurrentPosture = GCMovementComponent->GetCurrentPosture();
	if (GCMovementComponent->IsMovingOnGround() && (CurrentPosture == EPosture::Standing || CurrentPosture == EPosture::Crouching))
	{
		InverseKinematicsComponent->CalculateIkData(GetMesh(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			GetActorLocation(), bIsCrouched, DeltaTime);
	}

	UpdateSuffocatingState();
}

#pragma region ATTRIBUTES

void AGCBaseCharacter::OnOutOfHealth()
{
	bool bDeathMontagePlaying = false;
	InterruptReloading();
	StopAnimMontage();
	if (GCMovementComponent->IsMovingOnGround())
	{
		const float Duration = PlayAnimMontage(DeathAnimationMontage);
		bDeathMontagePlaying = Duration > 0.f;
	}

	GCMovementComponent->DisableMovement();
	SetInputDisabled(true, true);
	if (!bDeathMontagePlaying)
	{
		EnableRagdoll();
	}
}

void AGCBaseCharacter::UpdateSuffocatingState()
{
	if (GCMovementComponent->IsSwimming())
	{
		FVector HeadPosition = GetMesh()->GetBoneLocation(HeadBoneName);
		APhysicsVolume* Volume = GCMovementComponent->GetPhysicsVolume();
		if (IsValid(Volume))
		{
			float WaterPlaneZ = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
			CharacterAttributesComponent->SetSuffocating(HeadPosition.Z < WaterPlaneZ);
		}
	}
}

void AGCBaseCharacter::OnOutOfStamina(bool bOutOfStamina)
{
	if (bOutOfStamina)
	{
		if (GCMovementComponent->IsSprinting())
		{
			StopSprinting();
			SetStrafingControlsState(false);
		}
		else if (GCMovementComponent->IsWallrunning())
		{
			StopWallrunning();
		}
	}
	
	GCMovementComponent->SetIsOutOfStamina(bOutOfStamina);
}

#pragma endregion ATTRIBUTES

#pragma region SPRINT

void AGCBaseCharacter::StartRequestingSprint()
{
	bSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AGCBaseCharacter::StopRequestingSprint()
{
	bSprintRequested = false;
}

bool AGCBaseCharacter::CanSprint() const
{
	return IsPendingMovement()
		&& !bAiming
		&& GCMovementComponent->IsMovingOnGround() && !CharacterAttributesComponent->IsOutOfStamina()
		&& GCMovementComponent->GetCurrentPosture() != EPosture::Proning;
}

void AGCBaseCharacter::TryStartSprinting()
{
	bool bSprintStarted = GCMovementComponent->TryStartSprint();
	if (bSprintStarted && CharacterEquipmentComponent->GetEquippedItemType() != EEquippableItemType::None)
	{
		SetStrafingControlsState(false);
	}
	
	CharacterAttributesComponent->SetSprinting(bSprintStarted);
	OnSprintStart();
}

void AGCBaseCharacter::StopSprinting()
{
	if (!GCMovementComponent->IsSprinting())
	{
		return;
	}
	
	GCMovementComponent->StopSprint();
	if (GCMovementComponent->IsMovingOnGround() && CharacterEquipmentComponent->IsPreferStrafing())
	{
		SetStrafingControlsState(true);
	}
	
	CharacterAttributesComponent->SetSprinting(false);
	OnSprintEnd();
}

void AGCBaseCharacter::TryChangeSprintState()
{
	if (bSprintRequested && !GCMovementComponent->IsSprinting() && CanSprint())
	{
		TryStartSprinting();
	}
	else if ((!bSprintRequested || !CanSprint()) && GCMovementComponent->IsSprinting())
	{
		StopSprinting();
	}
}

#pragma endregion 

#pragma region JUMP/FALL

void AGCBaseCharacter::Jump()
{
	if (GCMovementComponent->GetCurrentPosture() == EPosture::Proning)
	{
		GCMovementComponent->TryStandUp();
	}
	else if (bIsCrouched)
	{
		UnCrouch();
	}
	else if (GCMovementComponent->IsClimbing())
	{
		GCMovementComponent->StopClimbing(EStopClimbingMethod::JumpOff);
	}
	else if (GCMovementComponent->IsZiplining())
	{
		StopZiplining();
	}
	else if (GCMovementComponent->IsWallrunning())
	{
		GCMovementComponent->JumpOffWall();
	}
	else
	{
		if (GCMovementComponent->IsSliding())
		{
			StopSliding();
		}
		
		Super::Jump();
	}
}

void AGCBaseCharacter::OnJumped_Implementation()
{
	CharacterAttributesComponent->OnJumped();
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation()
		&& !CharacterAttributesComponent->IsOutOfStamina()
		&& !GCMovementComponent->IsMantling()
		&& !GCMovementComponent->IsProning();
}

void AGCBaseCharacter::Falling()
{
	if (GCMovementComponent->IsSprinting())
	{
		StopSprinting();
	}

	CharacterAttributesComponent->SetFalling(true);
}

void AGCBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	GCMovementComponent->bNotifyApex = 1;
	const FVector Velocity = GCMovementComponent->Velocity;

	if (Hit.bBlockingHit && -Velocity.Z > HardLandVelocityZ)
	{
		const float SurfaceNormalZToStopSliding = 0.9f;
		if (GCMovementComponent->IsSliding() && Hit.ImpactNormal.Z > SurfaceNormalZToStopSliding)
		{
			StopSliding(true);
		}

		FMontagePlayResult MontagePlayResult = PlayHardLandMontage();
		if (MontagePlayResult.bStarted)
		{
			GCMovementComponent->Velocity = FVector::ZeroVector;
			SetInputDisabled(true, MontagePlayResult.bDisableCameraRotation);
			MontagePlayResult.AnimInstance->Montage_SetEndDelegate(MontageEndedUnlockControlsEvent, MontagePlayResult.AnimMontage);	
		}
	}

	if (IsValid(FallDamageCurve))
	{
		float FallDamage = FallDamageCurve->GetFloatValue(-Velocity.Z);
		if (FallDamage > 0.f)
		{
			TakeDamage(FallDamage, FDamageEvent(), GetController(), Hit.Actor.Get());
		}
	}
	
	CharacterAttributesComponent->SetFalling(false);
}

void AGCBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	FallApexWorldZ = GetActorLocation().Z;
}

FMontagePlayResult AGCBaseCharacter::PlayHardLandMontage()
{
	InterruptReloading();
	return PlayHardLandMontage(GetMesh()->GetAnimInstance(), HardLandMontageTP);
}

FMontagePlayResult AGCBaseCharacter::PlayHardLandMontage(UAnimInstance* AnimInstance, UAnimMontage* AnimMontage) const
{
	FMontagePlayResult MontagePlayResult;
	MontagePlayResult.bStarted = false;
	if (IsValid(AnimInstance))
	{
		if (IsValid(AnimMontage))
		{
			MontagePlayResult.bStarted = AnimInstance->Montage_Play(AnimMontage, 1, EMontagePlayReturnType::Duration, 0.f) > 0;
			if (MontagePlayResult.bStarted)
			{
				MontagePlayResult.AnimInstance = AnimInstance;
				MontagePlayResult.AnimMontage = AnimMontage;
			}
		}
	}

	return MontagePlayResult;
}

#pragma endregion JUMP/FALL

#pragma region INTERACTION

void AGCBaseCharacter::Interact()
{
	if (bInteracting)
	{
		TryStopInteracting();
	}
	else
	{
		TryStartInteracting();
		if (bInteracting)
		{
			InterruptReloading();
		}
	}
}

void AGCBaseCharacter::TryStopInteracting()
{
	if (IsValid(CurrentInteractable))
	{
		switch (CurrentInteractable->GetInteractionType())
		{
			case EInteractionType::Climbing:
				GCMovementComponent->StopClimbing();
				break;
			case EInteractionType::Ziplining:
				break;
			default:
				return;
		}	
	}

	ResetInteraction();
}

// TODO InteractionComponent
bool AGCBaseCharacter::TryStartInteracting()
{
	const AInteractiveActor* Interactable = GetPreferableInteractable();
	if (!IsValid(Interactable))
	{
		return false;
	}

	bool bInteractionStarted = false;
	switch (Interactable->GetInteractionType())
	{
		case EInteractionType::Climbing:
		{
			const ALadder* Ladder = StaticCast<const ALadder*>(Interactable);

			if (Ladder->IsOnTop())
			{
				PlayAnimMontage(Ladder->GetAttachFromTopAnimMontage());
			}
	
			bInteractionStarted = GCMovementComponent->TryStartClimbing(Ladder);
			break;
		}
		case EInteractionType::Ziplining:
		{
			const AZipline* Zipline = StaticCast<const AZipline*>(Interactable);
			FZiplineParams ZiplineParams = GetZipliningParameters(Zipline);
			bInteractionStarted = GCMovementComponent->TryStartZiplining(ZiplineParams);
			break;
		}
		default:
			break;
	}

	if (bInteractionStarted)
	{
		bInteracting = true;
		CurrentInteractable = Interactable;	
	}

	return bInteractionStarted;
}

void AGCBaseCharacter::RegisterInteractiveActor(const AInteractiveActor* InteractiveActor)
{
	InteractiveActor->Tags;
	InteractiveActors.AddUnique(InteractiveActor);
}

void AGCBaseCharacter::UnregisterInteractiveActor(const AInteractiveActor* InteractiveActor)
{
	if (InteractiveActor != CurrentInteractable)
		InteractiveActors.RemoveSingleSwap(InteractiveActor);
}

const AInteractiveActor* AGCBaseCharacter::GetPreferableInteractable()
{
	auto InteractablesCount = InteractiveActors.Num();	
	if (InteractablesCount <= 0)
		return nullptr;

	const AInteractiveActor* Interactable = nullptr;
	while (InteractablesCount > 0)
	{
		Interactable = InteractiveActors[0];
		if (IsValid(Interactable))
			break;
		
		InteractiveActors.RemoveSingleSwap(CurrentInteractable);
		Interactable = nullptr;
		InteractablesCount--;
	}
	
	return Interactable;
}

void AGCBaseCharacter::ResetInteraction()
{
	bInteracting = false;
	CurrentInteractable = nullptr;
}

#pragma endregion INTERACTION

#pragma region CROUCH/PRONE

void AGCBaseCharacter::ToggleCrouchState()
{
	if (!bIsCrouched && !GCMovementComponent->IsSliding())
	{
		GCMovementComponent->TryCrouch();
	}
}

void AGCBaseCharacter::ToggleProneState()
{
	if (bIsCrouched)
	{
		GCMovementComponent->TryProne();
	}
}

bool AGCBaseCharacter::CanCrouch() const
{
	return Super::CanCrouch() && !GCMovementComponent->IsSwimming();
}

void AGCBaseCharacter::OnStartCrouchOrProne(float HalfHeightAdjust)
{
	UpdateStrafingControls();
	ChangeMeshOffset(HalfHeightAdjust);
}

void AGCBaseCharacter::OnEndCrouchOrProne(float HalfHeightAdjust)
{
	UpdateStrafingControls();
	ChangeMeshOffset(-HalfHeightAdjust);
}

#pragma endregion CROUCH/PRONE

#pragma region ZIPLINE

void AGCBaseCharacter::OnZiplineObstacleHit(FHitResult Hit)
{
	if (Hit.bBlockingHit)
	{
		StopZiplining();
	}
}

void AGCBaseCharacter::StopZiplining()
{
	GCMovementComponent->StopZiplining();
	ResetInteraction();
}

FZiplineParams AGCBaseCharacter::GetZipliningParameters(const AZipline* Zipline) const
{
	FZiplineParams ZiplineParams;
	ZiplineParams.Friction = Zipline->GetCableFriction();
	ZiplineParams.ZiplineNormalizedDirection = Zipline->GetZiplineDirection().GetSafeNormal();
	ZiplineParams.DeclinationAngle = Zipline->GetDeclinationAngle();
	const FVector TopPoleLocation = Zipline->GetTopPoleWorldLocation();
	const FVector BottomPoleLocation = Zipline->GetBottomPoleWorldLocation();
	const FVector TopPoleProjection = FVector(TopPoleLocation.X, TopPoleLocation.Y, BottomPoleLocation.Z); 	
	ZiplineParams.MovementPlane = FPlane(TopPoleLocation,BottomPoleLocation, TopPoleProjection);
				
	const float ZiplineDistance = (TopPoleLocation - BottomPoleLocation).Size();
	const FVector& ActorLocation = GetActorLocation();
	FVector ProjectedVelocity = FVector::VectorPlaneProject(GetVelocity(), ZiplineParams.MovementPlane.GetNormal());
	ZiplineParams.CurrentSpeed = ProjectedVelocity.Size();
	
	ZiplineParams.DeclinationAngleSin = FMath::Sin(FMath::DegreesToRadians(ZiplineParams.DeclinationAngle));
	ZiplineParams.DeclinationAngleCos = FMath::Cos(FMath::DegreesToRadians(ZiplineParams.DeclinationAngle));
	
	FVector SocketOffset = GetMesh()->GetSocketTransform(ZiplineHandPositionSocketName,RTS_Actor).GetLocation();
	const float CharacterToZiplineDistance = (TopPoleLocation - ActorLocation).Size();
	FVector NewActorLocation = TopPoleLocation
		+ (BottomPoleLocation - TopPoleLocation) * CharacterToZiplineDistance / ZiplineDistance;
	float RotationHeightDeltaAngle = (180.f - ZiplineParams.DeclinationAngle) * 0.5f;
	const UCapsuleComponent* CharacterCapsule = GetCapsuleComponent();
	const float HalfHeightAdjustment = (CharacterCapsule->GetScaledCapsuleHalfHeight() + CharacterCapsule->GetScaledCapsuleRadius())
	* ZiplineParams.DeclinationAngleSin * (1.f / FMath::Tan(FMath::DegreesToRadians(RotationHeightDeltaAngle)));
	ZiplineParams.CorrectedActorLocation = NewActorLocation - SocketOffset - HalfHeightAdjustment * FVector::UpVector;
	return ZiplineParams;
}

#pragma endregion ZIPLINE

#pragma region MANTLE

void AGCBaseCharacter::Mantle(bool bForce)
{
	if (!CanMantle() && !bForce)
	{
		return;
	}
	
	FLedgeDescriptor LedgeDescriptor;
	if (LedgeDetectionComponent->DetectLedge(LedgeDescriptor))
	{
		const auto ActorTransform = GetActorTransform();
		const auto MantleSettings = GetMantlingSettings(LedgeDescriptor.MantlingHeight);
		FMantlingMovementParameters MantleParams(LedgeDescriptor, ActorTransform, MantleSettings);
		if (bIsCrouched)
		{
			const float DefaultHalfHeight = GetDefaultHalfHeight() * GetActorScale().Z;
			MantleParams.InitialLocation.Z += DefaultHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
		
		bool bMantleStarted = GCMovementComponent->TryStartMantle(MantleParams);
		if (!bMantleStarted)
		{
			return;
		}
		else
		{
			InterruptReloading();
		}
		
		if (bIsCrouched)
		{
			UnCrouch();
		}
		
		PlayMantleMontage(MantleSettings, MantleParams.StartTime);
	}
}

void AGCBaseCharacter::PlayMantleMontage(const FMantlingSettings& MantleSettings, float StartTime)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(MantleSettings.MantleMontageTP))
	{
		AnimInstance->Montage_Play(MantleSettings.MantleMontageTP, 1, EMontagePlayReturnType::Duration, StartTime);
	}
}

const FMantlingSettings& AGCBaseCharacter::GetMantlingSettings(float Height) const
{
	return Height <= MantleLowMaxHeight ? MantleLowSettings : MantleHighSettings;
}

bool AGCBaseCharacter::CanMantle() const
{
	return !GCMovementComponent->IsMantling()
		&& (GCMovementComponent->GetCurrentPosture() == EPosture::Standing
			|| GCMovementComponent->GetCurrentPosture() == EPosture::Crouching)
		&& !GCMovementComponent->IsWallrunning()
		&& IsValid(MantleHighSettings.MantleCurve) && IsValid(MantleLowSettings.MantleCurve);
}

#pragma endregion MANTLE

#pragma region CLIMB

void AGCBaseCharacter::OnClimbableTopReached()
{
	Mantle(true);
	InteractiveActors.RemoveSingleSwap(CurrentInteractable);
	ResetInteraction();
}

void AGCBaseCharacter::OnStoppedClimbing(const AInteractiveActor* Interactable)
{
	if (bInteracting && Interactable == CurrentInteractable)
	{
		// TODO refactor so that these fields are updated in a single method
		ResetInteraction();
	}
}

#pragma endregion CLIMB

#pragma region WALLRUN

void AGCBaseCharacter::StartRequestingWallrun()
{	
	if (CanAttemptWallrun())
	{
		GCMovementComponent->RequestWallrunning();
	}
}

void AGCBaseCharacter::StopRequestingWallrun()
{
	GCMovementComponent->StopWallrunning(false);
}

bool AGCBaseCharacter::CanAttemptWallrun() const
{
	return IsPendingMovement()
		&& !CharacterAttributesComponent->IsOutOfStamina()
		&& (GCMovementComponent->IsMovingOnGround() || GCMovementComponent->IsFalling())
		&& GCMovementComponent->GetCurrentPosture() == EPosture::Standing;
}

void AGCBaseCharacter::OnWallrunBegin(ESide WallrunSide)
{
	CharacterAttributesComponent->SetWallrunning(true);
	InterruptReloading();
}

void AGCBaseCharacter::OnWallrunEnd(ESide WallrunSide) const
{
	CharacterAttributesComponent->SetWallrunning(false);
}

void AGCBaseCharacter::StopWallrunning()
{
	GCMovementComponent->StopWallrunning(false);
	CharacterAttributesComponent->SetWallrunning(false);
}

#pragma endregion WALLRUN

#pragma region SLIDE

void AGCBaseCharacter::TryStartSliding()
{
	if (CanSlide())
	{
		bool bSlideStarted = GCMovementComponent->TryStartSliding();
		if (bSlideStarted)
		{
			InterruptReloading();
			StopSprinting();
		}
	}
}

void AGCBaseCharacter::StopSliding(bool bForce)
{
	if (bForce || GCMovementComponent->IsSliding())
	{
		GCMovementComponent->StopSliding();
	}
}

void AGCBaseCharacter::OnSlidingStateChangedEvent(bool bSliding, float HalfHeightAdjust)
{
	ChangeMeshOffset(bSliding ? HalfHeightAdjust : -HalfHeightAdjust);
}

#pragma endregion SLIDE

#pragma region WEAPONS

bool AGCBaseCharacter::CanShoot() const
{
	return !(GCMovementComponent->IsSwimming() || GCMovementComponent->IsClimbing()
		|| GCMovementComponent->IsZiplining() || GCMovementComponent->IsWallrunning()
		|| GCMovementComponent->IsMantling());
}

void AGCBaseCharacter::StartFiring()
{
	if (CharacterAttributesComponent->IsAlive() && CanShoot())
	{
		ARangeWeaponItem* RangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
		if (IsValid(RangeWeapon))
		{
			RangeWeapon->TryStartFiring(Controller);
		}
	}
}

void AGCBaseCharacter::StopFiring()
{
	CharacterEquipmentComponent->StopFiring();
}

void AGCBaseCharacter::StartAiming()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (!IsValid(CurrentRangeWeapon) || !CanAim())
	{
		return;
	}
	
	bAiming = true;
	StopSprinting();
	GCMovementComponent->SetIsAiming(true);
	CurrentRangeWeapon->StartAiming();
	OnAimingStart(CurrentRangeWeapon->GetAimFOV(), CurrentRangeWeapon->GetAimTurnModifier(), CurrentRangeWeapon->GetAimLookUpModifier());
	AimingStateChangedEvent.ExecuteIfBound(true);
}

void AGCBaseCharacter::StopAiming()
{
	if (!bAiming)
	{
		return;
	}
	
	bAiming = false;
	GCMovementComponent->SetIsAiming(false);
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopAiming();
	}
	
	OnAimingEnd();
	AimingStateChangedEvent.ExecuteIfBound(false);
}

void AGCBaseCharacter::StartReloading()
{
	if (CanReload())
	{
		FReloadData ReloadData = CharacterEquipmentComponent->TryReload();
		if (ReloadData.bStarted && IsValid(ReloadData.CharacterReloadMontage))
		{
			const float ActualDuration = PlayAnimMontageWithDuration(ReloadData.CharacterReloadMontage, ReloadData.DesiredDuration);
			if (ActualDuration > 0)
			{
				ActiveReloadMontage = ReloadData.CharacterReloadMontage;
			}
		}
	}
}

void AGCBaseCharacter::PickWeapon(int Delta)
{
	InterruptReloading();
	CharacterEquipmentComponent->EquipItem(Delta);
}

void AGCBaseCharacter::InterruptReloading()
{
	CharacterEquipmentComponent->InterruptReloading();
	if (IsValid(ActiveReloadMontage))
	{
		StopAnimMontage(ActiveReloadMontage);
		ActiveReloadMontage = nullptr;
	}
}

bool AGCBaseCharacter::CanReload() const
{
	return (GCMovementComponent->IsMovingOnGround() || GCMovementComponent->IsFalling() || GCMovementComponent->IsProning())
	&& !CharacterEquipmentComponent->IsChangingEquipment();
}

void AGCBaseCharacter::OnShot(UAnimMontage* AnimMontage)
{
	InterruptReloading();
	PlayAnimMontage(AnimMontage);
}

void AGCBaseCharacter::OnChangingEquippedItemStarted(UAnimMontage* AnimMontage, float Duration)
{
	InterruptReloading();
	PlayAnimMontageWithDuration(AnimMontage, Duration);
}

bool AGCBaseCharacter::CanAim() const
{
	return !bAiming && GCMovementComponent->IsMovingOnGround();
}

void AGCBaseCharacter::OnWeaponPickedUpChanged(class ARangeWeaponItem* EquippedWeapon, bool bPickedUp)
{
	if (bPickedUp)
	{
		EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, EquippedWeapon->GetCharacterUnequippedSocketName());
		EquippedWeapon->ShootEvent.AddUObject(this, &AGCBaseCharacter::OnShot);
		// oh oh
		EquippedWeapon->AmmoChangedEvent.BindUObject(CharacterEquipmentComponent, &UCharacterEquipmentComponent::OnAmmoChanged);
		EquippedWeapon->OutOfAmmoEvent.BindLambda([this](){ if (CharacterEquipmentComponent->IsAutoReload()) StartReloading(); });
	}
	else
	{
		// FDelegateHandle instead of this?
		EquippedWeapon->ShootEvent.RemoveAll(this);
		EquippedWeapon->OutOfAmmoEvent.Unbind();
	}
}

void AGCBaseCharacter::OnWeaponEquippedChanged(ARangeWeaponItem* EquippedWeapon, bool bEquipped)
{
	FName SocketName;
	if (bEquipped)
	{
		GCMovementComponent->SetAimingSpeed(EquippedWeapon->GetAimMaxSpeed());
		SocketName = EquippedWeapon->GetCharacterEquippedSocketName();
		UpdateStrafingControls();
	}
	else
	{
		SocketName = EquippedWeapon->GetCharacterUnequippedSocketName();
	}

	EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, SocketName);
}

#pragma endregion WEAPONS

#pragma region MOVEMENT

void AGCBaseCharacter::MoveForward(float Value)
{
	CurrentInputForward = Value;
	GCMovementComponent->SetForwardInput(Value);
}

void AGCBaseCharacter::SetInputDisabled(bool bDisabledMovement, bool bDisabledCamera)
{
	if (IsValid(Controller))
	{
		Controller->SetIgnoreLookInput(bDisabledCamera);
		Controller->SetIgnoreMoveInput(bDisabledMovement);
		bMovementInputEnabled = !bDisabledMovement;
		if (bDisabledMovement)
		{
			CurrentInputForward = 0.f;
			CurrentInputRight = 0.f;
		}
	}
}

void AGCBaseCharacter::OnMontageEndedUnlockControls(UAnimMontage* AnimMontage, bool bInterrupted)
{
	SetInputDisabled(false, false);
	UpdateStrafingControls();
}

void AGCBaseCharacter::OnMantleEnded()
{
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
}

void AGCBaseCharacter::EnableRagdoll() const
{
	GetMesh()->SetCollisionProfileName(ProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}

void AGCBaseCharacter::UpdateStrafingControls()
{
	SetStrafingControlsState(GCMovementComponent->IsMovingOnGround() && CharacterEquipmentComponent->IsPreferStrafing()
		&& !CharacterAttributesComponent->IsOutOfStamina() && GCMovementComponent->GetCurrentPosture() == EPosture::Standing
		&& !GCMovementComponent->IsSprinting());
}

void AGCBaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	CharacterAttributesComponent->SetMovementMode(GCMovementComponent->MovementMode);
	if (PrevMovementMode == EMovementMode::MOVE_Swimming)
	{
		CharacterAttributesComponent->SetSuffocating(false);
	}

	UpdateStrafingControls();
}

void AGCBaseCharacter::SetStrafingControlsState(bool bStrafing)
{
	GCMovementComponent->bOrientRotationToMovement = !bStrafing;
	bUseControllerRotationYaw = bStrafing;
}

#pragma endregion MOVEMENT

void AGCBaseCharacter::ChangeMeshOffset(float HalfHeightAdjust)
{
	RecalculateBaseEyeHeight();
	FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
	MeshRelativeLocation.Z += HalfHeightAdjust;
	BaseTranslationOffset.Z = MeshRelativeLocation.Z;
}

float AGCBaseCharacter::PlayAnimMontageWithDuration(UAnimMontage* Montage, float DesiredDuration)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	const float PlayRate = Montage->GetPlayLength() / DesiredDuration;
	return AnimInstance->Montage_Play(Montage, PlayRate);
}

