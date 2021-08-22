// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"

#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameCode/Actors/Interactive/InteractiveActor.h"
#include "GameCode/Actors/Interactive/Environment/Ladder.h"
#include "GameCode/Actors/Interactive/Environment/Zipline.h"
#include "GameCode/Components/InverseKinematicsComponent.h"
#include "GameCode/Components/LedgeDetectionComponent.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"
#include "GameCode/Data/ZiplineParams.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	GCMovementComponent->ClimbableTopReached.BindUObject(this, &AGCBaseCharacter::OnClimbableTopReached);
	GCMovementComponent->StoppedClimbing.BindUObject(this, &AGCBaseCharacter::OnStoppedClimbing);
	GCMovementComponent->CrouchedOrProned.BindUObject(this, &AGCBaseCharacter::OnStartCrouchOrProne);
	GCMovementComponent->WokeUp.BindUObject(this, &AGCBaseCharacter::OnEndCrouchOrProne);
	GCMovementComponent->ZiplineObstacleHit.BindUObject(this, &AGCBaseCharacter::OnZiplineObstacleHit);
}

// Called when the game starts or when spawned
void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentStamina = MaxStamina;
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateStamina(DeltaTime);
	TryChangeSprintState();
	const EPosture CurrentPosture = GCMovementComponent->GetCurrentPosture();
	if (GCMovementComponent->IsMovingOnGround() && (CurrentPosture == EPosture::Standing || CurrentPosture == EPosture::Crouching))
	{
		InverseKinematicsComponent->CalculateIkData(GetMesh(), GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
			GetActorLocation(), bIsCrouched, DeltaTime);
	}
}

#pragma region STAMINA

void AGCBaseCharacter::UpdateStamina(float DeltaTime)
{
	const float CurrentStaminaConsumption = GetCurrentStaminaConsumption(DeltaTime);
	if (CurrentStaminaConsumption > 0.f)
	{
		ChangeStaminaValue(-CurrentStaminaConsumption * DeltaTime);
	}
	else if (CanRestoreStamina())
	{
		ChangeStaminaValue(StaminaRestoreVelocity * DeltaTime);
	}
}

float AGCBaseCharacter::GetCurrentStaminaConsumption(float DeltaTime) const
{
	if (GCMovementComponent->IsSprinting())
	{
		return SprintStaminaConsumptionRate;
	}
	else if (GCMovementComponent->IsWallrunning())
	{
		return SprintStaminaConsumptionRate;
	}
	
	return 0.f;
}

void AGCBaseCharacter::ChangeStaminaValue(float StaminaModification)
{
	CurrentStamina = FMath::Clamp(CurrentStamina + StaminaModification, 0.f, MaxStamina);

	if (CurrentStamina == 0.f && !GCMovementComponent->IsOutOfStamina())
	{
		// TODO refactor so that all 3 stprint stoppage occur in a single method
		if (GCMovementComponent->IsSprinting())
		{
			StopSprint();
			OnSprintEnd();
		}
		else if (GCMovementComponent->IsWallrunning())
		{
			GCMovementComponent->StopWallrunning(false);
		}
		GCMovementComponent->SetIsOutOfStamina(true);
	}
	else if (CurrentStamina == MaxStamina && GCMovementComponent->IsOutOfStamina())
	{
		GCMovementComponent->SetIsOutOfStamina(false);
	}

#ifdef UE_BUILD_DEBUG
	const FColor LogColor = FLinearColor::LerpUsingHSV( FLinearColor::Red, FLinearColor::Yellow,
		CurrentStamina / MaxStamina).ToFColor(false);
	GEngine->AddOnScreenDebugMessage(1, 1.0f, LogColor, FString::Printf(TEXT("Stamina: %.2f"),
		CurrentStamina));
#endif
}

bool AGCBaseCharacter::CanRestoreStamina() const
{
	return !GCMovementComponent->IsFalling() && !IsConsumingStamina();
}

bool AGCBaseCharacter::IsConsumingStamina() const
{
	return !GCMovementComponent->IsMovingOnGround() || GCMovementComponent->IsSprinting();
}

#pragma endregion STAMINA

#pragma region SPRINT

void AGCBaseCharacter::StartSprint()
{
	bSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AGCBaseCharacter::StopSprint()
{
	bSprintRequested = false;
}

bool AGCBaseCharacter::CanSprint() const
{
	return IsPendingMovement()
		&& GCMovementComponent->IsMovingOnGround() && !GCMovementComponent->IsOutOfStamina()
		&& GCMovementComponent->GetCurrentPosture() != EPosture::Proning;
}

void AGCBaseCharacter::TryChangeSprintState()
{
	if (bSprintRequested && !GCMovementComponent->IsSprinting() && CanSprint())
	{
		GCMovementComponent->TryStartSprint();
		OnSprintStart();
	}
	else if ((!bSprintRequested || !IsPendingMovement()) && GCMovementComponent->IsSprinting())
	{
		GCMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

void AGCBaseCharacter::OnSprintEnd_Implementation()
{
}

void AGCBaseCharacter::OnSprintStart_Implementation()
{
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
		ChangeStaminaValue(JumpStaminaConsumption);
	}
	else
	{
		Super::Jump();
	}
}

void AGCBaseCharacter::OnJumped_Implementation()
{
	ChangeStaminaValue(-JumpStaminaConsumption);
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation()
		&& !GCMovementComponent->IsOutOfStamina()
		&& !GCMovementComponent->IsMantling()
		&& !GCMovementComponent->IsProning();
}

void AGCBaseCharacter::Falling()
{
	if (GCMovementComponent->IsSprinting())
	{
		GCMovementComponent->StopSprint();
		OnSprintEnd();
	}
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
void AGCBaseCharacter::TryStartInteracting()
{
	const AInteractiveActor* Interactable = GetPreferableInteractable();
	if (!IsValid(Interactable))
	{
		return;
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
	RecalculateBaseEyeHeight();
	FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
	MeshRelativeLocation.Z += HalfHeightAdjust;
	BaseTranslationOffset.Z = MeshRelativeLocation.Z;
}

void AGCBaseCharacter::OnEndCrouchOrProne(float HalfHeightAdjust)
{
	RecalculateBaseEyeHeight();
	FVector& MeshRelativeLocation = GetMesh()->GetRelativeLocation_DirectMutable();
	MeshRelativeLocation.Z -= HalfHeightAdjust;// GetDefault<AGCBaseCharacter>(GetClass())->GetMesh()->GetRelativeLocation().Z;
	BaseTranslationOffset.Z = MeshRelativeLocation.Z;
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
		
		if (bIsCrouched)
		{
			UnCrouch();
		}
		
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantleSettings.MantleMontage, 1, EMontagePlayReturnType::Duration,
			MantleParams.StartTime);
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
		&& !GCMovementComponent->IsWallrunning();
}

#pragma endregion MANTLE

#pragma region CLIMB

void AGCBaseCharacter::OnClimbableTopReached()
{
	Mantle(true);
	bInteracting = false;
	InteractiveActors.RemoveSingleSwap(CurrentInteractable);
	CurrentInteractable = nullptr;
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

void AGCBaseCharacter::StartWallrun()
{
	if (CanAttemptWallrun())
	{
		GCMovementComponent->RequestWallrunning();
	}
}

void AGCBaseCharacter::StopWallrun()
{
	GCMovementComponent->StopWallrunning(false);
}

bool AGCBaseCharacter::CanAttemptWallrun() const
{
	return IsPendingMovement()
		&& CurrentStamina > 0.f
		&& (GCMovementComponent->IsMovingOnGround() || GCMovementComponent->IsFalling())
		&& GCMovementComponent->GetCurrentPosture() == EPosture::Standing;
}

#pragma endregion WALLRUN

#pragma region SLIDE

void AGCBaseCharacter::TryStartSliding()
{
	if (CanSlide())
	{
		GCMovementComponent->TryStartSliding();
	}
}

void AGCBaseCharacter::StopSliding()
{
	if (GCMovementComponent->IsSliding())
	{
		GCMovementComponent->StopSliding();
	}
}

#pragma endregion SLIDE

void AGCBaseCharacter::MoveForward(float Value)
{
	CurrentInputForward = Value;
	GCMovementComponent->SetForwardInput(Value);
}
