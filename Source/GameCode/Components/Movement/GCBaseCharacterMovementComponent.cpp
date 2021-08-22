// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "GameCode/GameCode.h"
#include "GameCode/GCDebugSubsystem.h"
#include "GameCode/Actors/Interactive/Environment/Ladder.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Data/GCMovementMode.h"
#include "GameCode/Data/SlideData.h"
#include "GameCode/Data/WakeUpParams.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UGCBaseCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	DefaultWalkSpeed = MaxWalkSpeed;
	GCCharacter = Cast<AGCBaseCharacter>(CharacterOwner);
	CharacterOwner->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UGCBaseCharacterMovementComponent::OnPlayerCapsuleHit);
	PreparePostureHalfHeights();
}

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	if (bOutOfStamina)
	{
		return OutOfStaminaSpeed;
	}
	else if (bSprinting)
	{
		return SprintSpeed;
	}
	else if (IsClimbing())
	{
		return MaxClimbingSpeed;
	}
	else if (CurrentPosture == EPosture::Proning)
	{
		return CrawlSpeed;	
	}
	else if (IsWallrunning())
	{
		return WallrunSettings.MaxSpeed;
	}
	else if (IsSliding())
	{
		return SlideSettings.MaxSpeed;
	}

	return Super::GetMaxSpeed();
}

bool UGCBaseCharacterMovementComponent::TryStartSprint()
{
	if (CurrentPosture != EPosture::Standing)
	{
		if (CurrentPosture == EPosture::Crouching)
		{
			if (!TryWakeUpToState(EPosture::Standing))
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	
	bSprinting = true;
	bForceMaxAccel = 1;
	return true;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bSprinting = false;
	bForceMaxAccel = 0;
}

void UGCBaseCharacterMovementComponent::SetIsOutOfStamina(bool bNewOutOfStamina)
{
	bOutOfStamina = bNewOutOfStamina;
	MaxWalkSpeed = bNewOutOfStamina ? OutOfStaminaSpeed : DefaultWalkSpeed;
	if (bNewOutOfStamina && bSprinting)
	{
		StopSprint();
	}
}

void UGCBaseCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
	{
		if (bWantsToCrouch && CurrentPosture != EPosture::Crouching)
		{
			Crouch();
		}
		else if (bWantsToProne && CurrentPosture != EPosture::Proning)
		{
			Prone();
		}
		else if (!bWantsToCrouch && CurrentPosture == EPosture::Crouching)
		{
			UnCrouch();
		}
		else if (!bWantsToProne && CurrentPosture == EPosture::Proning)
		{
			UnProne();
		}
	}
}

void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (MovementMode == MOVE_Swimming)
	{
		if (IsCrouching())
		{
			UnCrouch();
		}
		else if (IsProning())
		{
			UnProne();
		}
		else if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)EGCMovementMode::CMOVE_Slide)
		{
			ResetFromSliding();
		}

		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		const UCapsuleComponent* DefaultCharacterCapsule = CharacterOwner->GetClass()
			->GetDefaultObject<ACharacter>()->GetCapsuleComponent();
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacterCapsule->GetUnscaledCapsuleRadius(),
			DefaultCharacterCapsule->GetUnscaledCapsuleHalfHeight(),true);
		FRotator Rotation = CharacterOwner->GetActorRotation();
		Rotation.Roll = 0;
		CharacterOwner->SetActorRotation(Rotation);
	}
	else if (PreviousMovementMode == MOVE_Custom)
	{
		switch (PreviousCustomMode)
		{
		case EGCMovementMode::CMOVE_Climbing:
			StoppedClimbing.ExecuteIfBound(CurrentClimbable);
			CurrentClimbable = nullptr;
			break;
		default:
			break;
		}
	}

	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case EGCMovementMode::CMOVE_Mantling:
			GetWorld()->GetTimerManager().SetTimer(MantlingTimerHandle, this,
				&UGCBaseCharacterMovementComponent::EndMantle, MantlingParameters.Duration);
			break;
		case EGCMovementMode::CMOVE_Climbing:
		default:
			break;
		}
	}
}

#pragma region CROUCH

void UGCBaseCharacterMovementComponent::TryStandUp()
{
	bWantsToProne = false;
	bWantsToCrouch = false;
}

void UGCBaseCharacterMovementComponent::TryCrouch()
{
	bWantsToProne = false;
	bWantsToCrouch = true;
}

void UGCBaseCharacterMovementComponent::Crouch(bool bClientSimulation)
{
	if (!bClientSimulation && !CanCrouchInCurrentState() || !IsMovingOnGround() || IsSprinting())
	{
		return;
	}
	
	const auto DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	if (TryCrouchOrProne(CrouchedHalfHeight, DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius()))
	{
		CurrentPosture = EPosture::Crouching;
		// IDK if I can get rid of this since i'm using postures now
		CharacterOwner->bIsCrouched = true; 
	}
	else
	{
		bWantsToCrouch = false;
	}
}

void UGCBaseCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
	TryWakeUpToState(EPosture::Standing);
}

#pragma endregion CROUCH

#pragma region PRONE

void UGCBaseCharacterMovementComponent::Prone()
{
	if (!CanProne())
	{
		bWantsToProne = false;
		return;
	}

	if (TryCrouchOrProne(ProneCapsuleHalfHeight, ProneCapsuleRadus))
	{
		CurrentPosture = EPosture::Proning;
		CharacterOwner->bIsCrouched = false;
	}
	else
	{
		bWantsToProne = false;
	}
}

void UGCBaseCharacterMovementComponent::UnProne()
{
	if (CanUnprone())
	{
		TryWakeUpToState(EPosture::Standing);
	}
}

// TODO refactor with bClientSimulation
bool UGCBaseCharacterMovementComponent::TryCrouchOrProne(float NewCapsuleHalfHeight, float NewCapsuleRadius)
{
	// See if collision is already at desired size.
	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == NewCapsuleHalfHeight)
	{
		CrouchedOrProned.ExecuteIfBound(0);
		return true;
	}

	const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	// Height is not allowed to be smaller than radius.
	const float ClampedNewHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, NewCapsuleHalfHeight);

	float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedNewHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

	// when going to crouch state from proned
	if (ClampedNewHalfHeight > OldUnscaledHalfHeight)
	{
		FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
		FCollisionResponseParams ResponseParam;
		InitCollisionParams(CapsuleParams, ResponseParam);
		const bool IsBlocked = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f,0.f,ScaledHalfHeightAdjust), FQuat::Identity,
			UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

		if(IsBlocked)
		{
			return false;
		}
	}

	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(NewCapsuleRadius, ClampedNewHalfHeight);
	
	UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust),
		UpdatedComponent->GetComponentQuat(), true, nullptr,
		EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
	
	bForceNextFloorCheck = true;
	AdjustProxyCapsuleSize();
	CrouchedOrProned.ExecuteIfBound(ScaledHalfHeightAdjust);
	return true;
}

void UGCBaseCharacterMovementComponent::FillWakeUpParams(FWakeUpParams& WakeUpParams) const
{
	const auto CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	WakeUpParams.DefaultCharacter = DefaultCharacter;
	WakeUpParams.CapsuleComponent = CapsuleComponent;
	WakeUpParams.ComponentScale = CapsuleComponent->GetShapeScale();
	WakeUpParams.SweepInflation = KINDA_SMALL_NUMBER * 10.f;
	WakeUpParams.PawnLocation = UpdatedComponent->GetComponentLocation();
	WakeUpParams.CollisionQueryParams = FCollisionQueryParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
	InitCollisionParams(WakeUpParams.CollisionQueryParams, WakeUpParams.ResponseParam);
	WakeUpParams.CollisionChannel = UpdatedComponent->GetCollisionObjectType();
}

bool UGCBaseCharacterMovementComponent::TryWakeUpToState(EPosture DesiredPosture, bool bClientSimulation)
{
	if (CurrentPosture != DesiredPosture)
	{
		FWakeUpParams WakeUpParams;
		FillWakeUpParams(WakeUpParams);

		const float CurrentUnscaledHalfHeight = WakeUpParams.CapsuleComponent->GetUnscaledCapsuleHalfHeight();
		const float DesiredUnscaledHalfHeight = PostureCapsuleHalfHeights[DesiredPosture];
		for (const auto& heights : PostureCapsuleHalfHeights)
		{
			if (heights.Value > DesiredUnscaledHalfHeight)
			{
				continue;
			}

			// can't wake up to a collision shape smaller than current
			if (heights.Value < CurrentUnscaledHalfHeight)
			{
				break;
			}
			
			if (TryWakeUp(heights.Value, WakeUpParams, bClientSimulation))
			{
				CurrentPosture = heights.Key;
				break;
			}
		}
	}

	// TODO get rid of CharacterOwner->bIsCrouched completely?
	switch (CurrentPosture)
	{
		case EPosture::Crouching:
			bWantsToCrouch = true;
			CharacterOwner->bIsCrouched = true;
			bWantsToProne = false;
			break;
		case EPosture::Proning:
			bWantsToCrouch = false;
			CharacterOwner->bIsCrouched = false;
			bWantsToProne = true;
			break;
		case EPosture::Standing:
			bWantsToProne = false;
			CharacterOwner->bIsCrouched = false;
			bWantsToCrouch = false;
			break;
		default:
			break;
	}

	return CurrentPosture == DesiredPosture;
}

bool UGCBaseCharacterMovementComponent::TryWakeUp(float DesiredUnscaledHalfHeight, const FWakeUpParams& WakeUpParams, bool bClientSimulation)
{
	const float CurrentUnscaledHalfHeight = WakeUpParams.CapsuleComponent->GetUnscaledCapsuleHalfHeight();
	if(CurrentUnscaledHalfHeight == DesiredUnscaledHalfHeight)
	{
		WokeUp.ExecuteIfBound(0);
		return true;
	}
	
	const float CurrentHalfHeight = WakeUpParams.CapsuleComponent->GetScaledCapsuleHalfHeight();
	const float HalfHeightAdjust = DesiredUnscaledHalfHeight - CurrentUnscaledHalfHeight;
	const float ScaledHalfHeightAdjust = HalfHeightAdjust * WakeUpParams.ComponentScale;
	const UWorld* MyWorld = GetWorld();

	if (!bClientSimulation)
	{
		const FCollisionShape DesiredCapsuleShape = GetPawnCapsuleCollisionShape(SHRINK_HeightCustom,
			-WakeUpParams.SweepInflation - ScaledHalfHeightAdjust);
		FVector DesiredLocation = WakeUpParams.PawnLocation + FVector(0.f, 0.f,DesiredCapsuleShape.GetCapsuleHalfHeight() - CurrentHalfHeight);
		bool bCantWakeUp = MyWorld->OverlapBlockingTestByChannel(DesiredLocation, FQuat::Identity, WakeUpParams.CollisionChannel,
			DesiredCapsuleShape, WakeUpParams.CollisionQueryParams, WakeUpParams.ResponseParam);

		if (bCantWakeUp)
		{
			//looks redundant, was in ACharacter::UnCrouch so why not
			if (IsMovingOnGround())
			{
				// Something might be just barely overhead, try moving down closer to the floor to avoid it.
				const float MinFloorDist = KINDA_SMALL_NUMBER * 10.f;
				if (CurrentFloor.bBlockingHit && CurrentFloor.FloorDist > MinFloorDist)
				{
					DesiredLocation.Z -= CurrentFloor.FloorDist - MinFloorDist;
					bCantWakeUp = MyWorld->OverlapBlockingTestByChannel(DesiredLocation, FQuat::Identity,
						WakeUpParams.CollisionChannel, DesiredCapsuleShape, WakeUpParams.CollisionQueryParams, WakeUpParams.ResponseParam);
				}
			}				
		}

		if (bCantWakeUp)
		{
			return false;
		}

		UpdatedComponent->MoveComponent(DesiredLocation - WakeUpParams.PawnLocation, UpdatedComponent->GetComponentQuat(),
		false, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
		bForceNextFloorCheck = true;
	}
	else
	{
		bShrinkProxyCapsule = true;
	}

	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(WakeUpParams.DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
		DesiredUnscaledHalfHeight, true);

	AdjustProxyCapsuleSize();
	WokeUp.ExecuteIfBound(ScaledHalfHeightAdjust);
	return true;
}

bool UGCBaseCharacterMovementComponent::CanUnprone()
{
	return true;
}

bool UGCBaseCharacterMovementComponent::CanProne()
{
	return (IsMovingOnGround() || IsSliding()) && !UpdatedComponent->IsSimulatingPhysics(); 
}

void UGCBaseCharacterMovementComponent::TryProne()
{
	bWantsToProne = true;
	bWantsToCrouch = false;
}

#pragma endregion PRONE

#pragma region MANTLE

bool UGCBaseCharacterMovementComponent::TryStartMantle(const FMantlingMovementParameters& NewMantlingParameters)
{
	if (CustomMovementMode == (uint8)EGCMovementMode::CMOVE_Mantling)
		return false;
	
	this->MantlingParameters = NewMantlingParameters;
	Velocity = FVector::ZeroVector;
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)EGCMovementMode::CMOVE_Mantling);
	return  true;
}

void UGCBaseCharacterMovementComponent::EndMantle()
{
	SetMovementMode(MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom
		&& CustomMovementMode == (uint8)EGCMovementMode::CMOVE_Mantling;
}

#pragma endregion 

#pragma region CLIMB

float UGCBaseCharacterMovementComponent::GetActorToClimbableProjection(const ALadder* Ladder,
	const FVector& ActorLocation) const
{
	FVector LadderUpVector = Ladder->GetActorUpVector();
	FVector LadderToCharacterVector = ActorLocation - Ladder->GetActorLocation();
	return FVector::DotProduct(LadderUpVector, LadderToCharacterVector);
}

bool UGCBaseCharacterMovementComponent::TryStartClimbing(const ALadder* Ladder)
{
	if (IsClimbing())
		return false;
	
	CurrentClimbable = Ladder;
	
	FVector NewCharacterLocation;

	if (Ladder->IsOnTop())
	{
		NewCharacterLocation = Ladder->GetAttachFromTopAnimMontageStartingLocation();		
	}
	else
	{
		float CharacterToLadderProjection = GetActorToClimbableProjection(Ladder, GetActorLocation());
		NewCharacterLocation = Ladder->GetActorLocation()
			+ CharacterToLadderProjection * Ladder->GetActorUpVector()
			+ Ladder->GetActorForwardVector() * ClimbingOffset;	
	}
		
	CharacterOwner->SetActorLocation(NewCharacterLocation);

	FRotator TargetOrientationRotation = Ladder->GetActorForwardVector()
		.RotateAngleAxis(180, Ladder->GetActorUpVector()).ToOrientationRotator();
	CharacterOwner->SetActorRotation(TargetOrientationRotation);
	SetMovementMode(MOVE_Custom, (uint8)EGCMovementMode::CMOVE_Climbing);
	return true;
}

void UGCBaseCharacterMovementComponent::StopClimbing(EStopClimbingMethod StopClimbingMethod)
{
	switch (StopClimbingMethod)
	{
		case EStopClimbingMethod::ReachingTop:
			ClimbableTopReached.ExecuteIfBound();
			break;
		case EStopClimbingMethod::ReachingBottom:
			SetMovementMode(MOVE_Falling);
			break;
		case EStopClimbingMethod::JumpOff:
			{
				FVector JumpDirection = FVector::ZeroVector;
				float RightInput = 0;
				float UpInput = 0;
				if (GCCharacter.IsValid())
				{
					RightInput = GCCharacter->GetCurrentInputRight();
					UpInput = GCCharacter->GetCurrentInputForward();
				}
				
				if (UpInput >= 0)
				{
					FVector2D YawSourceRange(-1, 1);
					FVector2D YawTargetRange(-90, 90);
					float YawAngle = FMath::GetMappedRangeValueClamped(YawSourceRange, YawTargetRange,
						RightInput);
					FVector2D PitchSourceRange(0, 1);
					FVector2D PitchTargetRange(0, 45);
					float PitchAngle = FMath::GetMappedRangeValueClamped(PitchSourceRange, PitchTargetRange,
						UpInput);
					FRotator rotator(PitchAngle, YawAngle, 0);
					if (PitchAngle != 0 || YawAngle != 0)
						JumpDirection = rotator.RotateVector(CharacterOwner->GetActorForwardVector());
					else
						JumpDirection = -CharacterOwner->GetActorForwardVector();
				}
				else
				{
					JumpDirection = -CurrentClimbable->GetActorForwardVector();
				}
				
				Launch(JumpDirection * JumpOffClimbableSpeed + FVector(0, 0, 500));
				ForceTargetRotation = JumpDirection.ToOrientationRotator();
				bForceRotation = true;
				SetMovementMode(MOVE_Falling);
				break;
			}
		case EStopClimbingMethod::Fall:
		default:
			SetMovementMode(MOVE_Falling);
			break;
	}
}

bool UGCBaseCharacterMovementComponent::IsClimbing() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom
		&& CustomMovementMode == (uint8)EGCMovementMode::CMOVE_Climbing;
}

float UGCBaseCharacterMovementComponent::GetClimbingSpeedRatio() const
{
	checkf(IsValid(CurrentClimbable), TEXT("UGCBaseCharacterMovementComponent::GetClimbableSpeedRatio cannot be invoked without an active climbable"));
	FVector ClimbableUpVector = CurrentClimbable->GetActorUpVector();
	return  FVector::DotProduct(Velocity, ClimbableUpVector) / MaxClimbingSpeed;
}

#pragma endregion CLIMBING

#pragma region ZIPLINE

bool UGCBaseCharacterMovementComponent::TryStartZiplining(const FZiplineParams& NewZiplineParams)
{
	if (IsZiplining())
	{
		return false;		
	}
	
	this->ZiplineParams = NewZiplineParams;
	bForceRotation = true;
	ForceTargetRotation = NewZiplineParams.ZiplineNormalizedDirection.ToOrientationRotator();
	SetMovementMode(MOVE_Custom, (uint8)EGCMovementMode::CMOVE_Zipline);
	return true;
}

void UGCBaseCharacterMovementComponent::StopZiplining()
{
	SetMovementMode(MOVE_Falling);
}

bool UGCBaseCharacterMovementComponent::IsZiplining() const
{
	return IsInCustomMovementMode(EGCMovementMode::CMOVE_Zipline);
}

#pragma endregion ZIPLINE

#pragma region WALLRUN

void UGCBaseCharacterMovementComponent::OnPlayerCapsuleHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsSurfaceWallrunnable(Hit.ImpactNormal))
	{
		WallrunData.Progress = 0.f;
		return;
	}
	
	if (!WallrunData.bWantsToWallrun || IsWallrunning() || bOutOfStamina)
	{
		return;
	}

	if (Velocity.Z < WallrunSettings.MinVelocityZ || Velocity.Size() < WallrunSettings.MinRequiredSpeed)
	{
		return;
	}

	const FVector Normal = Hit.ImpactNormal;
	
	ESide Side = GetWallrunSideFromNormal(Normal);
	const FVector AverageNormal = GetWallrunSurfaceNormal(Side);
	if (AverageNormal == FVector::ZeroVector)
	{
		return;
	}

	WallrunData.Side = Side;
	const float AngleCos = FVector::DotProduct(CharacterOwner->GetActorUpVector(), AverageNormal);
	const float Angle = FMath::RadiansToDegrees(FMath::Acos(AngleCos));
	WallrunData.SurfaceRollAngle = WallrunData.GetSideModificator(Side) * (90.f-Angle);
	WallrunData.SurfaceNormal = Normal;
	WallrunData.InitialWorldZ = GetActorLocation().Z;
	WallrunBeginEvent.ExecuteIfBound(Side);
	WallrunData.ActiveHeightDynamicCurve = IsMovingOnGround()
		? WallrunSettings.WallrunHeightDynamicsFromGroundCurve
		: WallrunSettings.WallrunHeightDynamicsFromAirCurve;
	
	SetMovementMode(MOVE_Custom, (uint8)EGCMovementMode::CMOVE_WallRun);
}

ESide UGCBaseCharacterMovementComponent::GetWallrunSideFromNormal(const FVector& Normal) const
{
	const float HorizontalWallrunDotProductThreshold = 0.3f;
	const float dpSurfaceNormalRightVector = FVector::DotProduct(Normal, CharacterOwner->GetActorRightVector());
	if (dpSurfaceNormalRightVector > HorizontalWallrunDotProductThreshold)
	{
		return ESide::Left;
	}
	else if (dpSurfaceNormalRightVector < -HorizontalWallrunDotProductThreshold)
	{
		return ESide::Right;
	}

	return ESide::None;
}

bool UGCBaseCharacterMovementComponent::IsSurfaceWallrunnable(const FVector& SurfaceNormal) const
{
	return SurfaceNormal.Z >= -KINDA_SMALL_NUMBER && SurfaceNormal.Z < GetWalkableFloorZ();
}

FVector UGCBaseCharacterMovementComponent::GetWallrunSurfaceNormal(const ESide& Side, const FVector& CharacterLocationDelta) const
{
	const int SideModificator = WallrunData.GetSideModificator(Side);
#if ENABLE_DRAW_DEBUG
	bool bDebugEnabled = GetDebugSubsystem()->IsDebugCategoryEnabled(DebugCategoryWallrun);
#else
	bool bDebugEnabled = false;
#endif
	
	const float CharacterScaleZ = CharacterOwner->GetActorScale().Z;
	FVector FeetPosition = GetActorLocation() + CharacterLocationDelta
		- FVector::UpVector * WallrunSettings.FeetTraceActorZOffset * CharacterScaleZ;
	const FVector DirectionVector = CharacterOwner->GetActorRightVector() * SideModificator;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Wallrunnable);
	FHitResult FeetHit;
	EDrawDebugTrace::Type TraceDebug = bDebugEnabled ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	bool bFeetTouch = UKismetSystemLibrary::LineTraceSingle(this, FeetPosition,
		FeetPosition + DirectionVector * (WallrunSettings.WallDistance + CapsuleRadius), TraceType,
		false,TArray<AActor*>(), TraceDebug, FeetHit, true);
	if (!bFeetTouch)
	{
		return FVector::ZeroVector;
	}

	FHitResult HandHit;
	FVector HandPosition = GetActorLocation() + CharacterLocationDelta
		+ FVector::UpVector * WallrunSettings.HandTraceActorZOffset * CharacterScaleZ;

	// for inclined walls 
	const float HandTraceExtendFactor = 4.f;
	bool bHandTouch = UKismetSystemLibrary::LineTraceSingle(this, HandPosition,
		HandPosition + DirectionVector * (WallrunSettings.WallDistance * HandTraceExtendFactor + CapsuleRadius), TraceType,
		false,TArray<AActor*>(), TraceDebug, HandHit, true);

	if (!bHandTouch)
	{
		return FVector::ZeroVector;
	}

	return (FeetHit.Normal + HandHit.Normal).GetSafeNormal();
}

void UGCBaseCharacterMovementComponent::RequestWallrunning()
{
	WallrunData.bWantsToWallrun = true;
}

void UGCBaseCharacterMovementComponent::StopWallrunning(bool bResetTimer)
{
	if (!IsWallrunning())
	{
		return;
	}
	
	WallrunData.bWantsToWallrun = false;
	if (bResetTimer)
	{
		WallrunData.Progress = 0.f;
	}
	
	WallrunEndEvent.ExecuteIfBound(WallrunData.Side);
	SetMovementMode(MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsWallrunning() const
{
	return MovementMode == EMovementMode::MOVE_Custom && CustomMovementMode == (uint8) EGCMovementMode::CMOVE_WallRun;
}

void UGCBaseCharacterMovementComponent::JumpOffWall()
{
	// TODO make some UPROPERTYs for these constants
	FVector JumpOffDirection = WallrunData.SurfaceNormal * 750.f
		+ CharacterOwner->GetActorUpVector() * 500.f
		+ CharacterOwner->GetActorForwardVector() * 320.f;
	StopWallrunning(true);
	bForceRotation = true;
	ForceTargetRotation = JumpOffDirection.ToOrientationRotator();
	ForceTargetRotation.Roll = 0.f;
	ForceTargetRotation.Pitch = 0.f;
	Launch(JumpOffDirection);
}

#pragma endregion 

#pragma region SLIDING

bool UGCBaseCharacterMovementComponent::IsSliding() const
{
	return IsInCustomMovementMode(EGCMovementMode::CMOVE_Slide);
}

bool UGCBaseCharacterMovementComponent::TryStartSliding()
{
	if (IsSliding() || !SlideData.bCanSlide)
	{
		return false;
	}
	
	const float CharacterScaleZ = CharacterOwner->GetActorScale().Z;
	TryCrouchOrProne(SlideSettings.CapsuleHalfHeight * CharacterScaleZ,
		CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius());
	GetWorld()->GetTimerManager().SetTimer(SlideData.TimerHandle, this,
		&UGCBaseCharacterMovementComponent::StopSliding, SlideSettings.Duration);
	CurrentPosture = EPosture::Sliding;
	StopSprint();
	WallrunData.bWantsToWallrun = false;
	SlideData.Speed = Velocity.Size();
	SetMovementMode(MOVE_Custom, (uint8)EGCMovementMode::CMOVE_Slide);
	return true;
}

void UGCBaseCharacterMovementComponent::StopSliding()
{
	if (!IsSliding())
	{
		return;
	}

	ResetFromSliding();
	SetMovementMode(MOVE_Walking);
}

void UGCBaseCharacterMovementComponent::ResetFromSliding()
{
	TryWakeUpToState(EPosture::Standing);
	if (CurrentPosture == EPosture::Sliding)
	{
		Prone();
	}

	SlideData.FloorAngle = 0.f;
	SlideData.bCanSlide = false;
	GetWorld()->GetTimerManager().ClearTimer(SlideData.TimerHandle);
	GetWorld()->GetTimerManager().SetTimer(SlideData.CooldownTimerHandle, this,
		&UGCBaseCharacterMovementComponent::ResetSlide, SlideSettings.CooldownTime);
}

void UGCBaseCharacterMovementComponent::ResetSlide()
{
	SlideData.bCanSlide = true;
}

#pragma endregion SLIDING

void UGCBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	switch (CustomMovementMode)
	{
		case EGCMovementMode::CMOVE_Mantling:
			PhysCustomMantling(DeltaTime, Iterations);
			break;
		case EGCMovementMode::CMOVE_Climbing:
			PhysCustomClimbing(DeltaTime, Iterations);
			break;
		case EGCMovementMode::CMOVE_WallRun:
			PhysCustomWallRun(DeltaTime, Iterations);
			break;
		case EGCMovementMode::CMOVE_Zipline:
			PhysCustomZiplining(DeltaTime, Iterations);
			break;
		case EGCMovementMode::CMOVE_Slide:
			PhysCustomSliding(DeltaTime, Iterations);
		default:
			break;
	}
	
	Super::PhysCustom(DeltaTime, Iterations);
}

void UGCBaseCharacterMovementComponent::PhysCustomMantling(float DeltaTime, int32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimerHandle) + MantlingParameters.StartTime;
	FVector CurveValue = MantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);
	float PositionAlpha = CurveValue.X;
	float XYCorrectionAlpha = CurveValue.Y;
	float ZCorrectionAlpha = CurveValue.Z;
	FVector CorrectedInitialLocation = FMath::Lerp(MantlingParameters.InitialLocation,
		MantlingParameters.InitialAnimationLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(MantlingParameters.InitialLocation.Z,
		MantlingParameters.InitialAnimationLocation.Z, ZCorrectionAlpha);
	FVector MantleTagetDeltaLocation = IsValid(MantlingParameters.MantleTarget)
		? MantlingParameters.MantleTarget->GetActorLocation() - MantlingParameters.InitialTargetLocation
		: FVector::ZeroVector;
	FVector NextLocation = FMath::Lerp(CorrectedInitialLocation, MantlingParameters.TargetLocation + MantleTagetDeltaLocation, PositionAlpha);
	FRotator NextRotation = FMath::Lerp(MantlingParameters.InitialRotation, MantlingParameters.TargetRotation, PositionAlpha);
	FHitResult Hit;
	SafeMoveUpdatedComponent(NextLocation - GetActorLocation(), NextRotation, false, Hit);
}

void UGCBaseCharacterMovementComponent::PhysCustomClimbing(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.f, false, ClimbingBrakingDeceleration);
	FVector Delta = Velocity * DeltaTime;
	FHitResult Hit;

	if (HasAnimRootMotion())
	{
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}
	
	FVector NewPosition = GetActorLocation() + Delta;
	float NewPositionLadderProjection = GetActorToClimbableProjection(CurrentClimbable, NewPosition);
	GEngine->AddOnScreenDebugMessage(2, 5, FColor::Blue,
		FString::Printf(TEXT("Current climbable projection: %f"), NewPositionLadderProjection));
	if (NewPositionLadderProjection < ClimbingDetachBottomOffset && Delta.Z < 0)
	{
		StopClimbing(EStopClimbingMethod::ReachingBottom);
		return;
	}

	if (NewPositionLadderProjection > CurrentClimbable->GetHeight() - ClimbingDetachTopOffset)
	{
		StopClimbing(EStopClimbingMethod::ReachingTop);
		return;
	}
	
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UGCBaseCharacterMovementComponent::PhysCustomWallRun(float DeltaTime, int32 iterations)
{
	const float ForwardInputThreshold = 0.2f;
	if (CurrentForwardInput < ForwardInputThreshold)
	{
		StopWallrunning(false);
		return;
	}

	const FVector ActorUpVector = CharacterOwner->GetActorUpVector();
	float VerticalOffset = 0.f;
	FVector SurfaceNormal = FVector::ZeroVector;
	float CurrentSpeed = WallrunSettings.MaxSpeed;
	const float CurrentWallrunProgress = (WallrunData.Progress + DeltaTime) / WallrunSettings.MaxTime;
	if (IsValid(WallrunData.ActiveHeightDynamicCurve))
	{
		const float CurrentWallrunDeltaHeightFactor = WallrunData.ActiveHeightDynamicCurve->GetFloatValue(CurrentWallrunProgress);
		const float CurrentZ = GetActorLocation().Z;
		const float ExpectedDeltaZ = WallrunSettings.MaxDeltaHeight * CurrentWallrunDeltaHeightFactor;
		VerticalOffset = ExpectedDeltaZ - (CurrentZ - WallrunData.InitialWorldZ);
		WallrunData.HeightCurveValue = CurrentWallrunDeltaHeightFactor;
		SurfaceNormal = GetWallrunSurfaceNormal(WallrunData.Side,ActorUpVector * VerticalOffset);
	}
	
	if (SurfaceNormal == FVector::ZeroVector)
	{
		SurfaceNormal = GetWallrunSurfaceNormal(WallrunData.Side);
		VerticalOffset = 0.f;	
	}
	
	if (SurfaceNormal == FVector::ZeroVector)
	{
		StopWallrunning(false);
		return;
	}

	if (IsValid(WallrunSettings.WallrunSpeedDynamicsCurve))
	{
		const float WallrunSpeedMultiplier = WallrunSettings.WallrunSpeedDynamicsCurve->GetFloatValue(CurrentWallrunProgress);
		CurrentSpeed = FMath::Min(CurrentSpeed, CurrentSpeed * WallrunSpeedMultiplier);
	}
	
	WallrunData.SurfaceNormal = SurfaceNormal;
	FVector NormalVelocity = FVector::ZeroVector;
	switch (WallrunData.Side)
	{
		case ESide::Left:
			NormalVelocity = FVector::CrossProduct(SurfaceNormal, ActorUpVector).GetSafeNormal();
			break;
		case ESide::Right:
			NormalVelocity = FVector::CrossProduct(ActorUpVector, SurfaceNormal).GetSafeNormal();			
			break;
		default:
			break;
	}
	
	FHitResult Hit;
	Velocity = (NormalVelocity * CurrentSpeed + ActorUpVector * VerticalOffset) * CurrentForwardInput;
	NormalVelocity = (NormalVelocity * CurrentSpeed * DeltaTime + ActorUpVector * VerticalOffset) * CurrentForwardInput;
	FRotator Rotation = NormalVelocity.ToOrientationRotator();
	Rotation.Roll = WallrunData.SurfaceRollAngle;
	Rotation.Pitch = 0.f;
	SafeMoveUpdatedComponent(NormalVelocity, Rotation, true, Hit);
	if (Hit.bBlockingHit)
	{
		if (!IsSurfaceWallrunnable(Hit.ImpactNormal))
		{
			StopWallrunning(true);
			return;
		}
		
		const float dpSurfaceForwardVector = FVector::DotProduct(Hit.ImpactNormal, CharacterOwner->GetActorForwardVector());
		const float SurfaceForwardVectorCosThreshold = -0.85f;
		if (dpSurfaceForwardVector < SurfaceForwardVectorCosThreshold)
		{
			StopWallrunning(false);
		}
		else
		{
			const float HitCorrectionRatio = 2.f;
			SafeMoveUpdatedComponent(NormalVelocity + Hit.ImpactNormal * HitCorrectionRatio, Rotation, false, Hit);			
		}
	}
	
	WallrunData.Progress += DeltaTime;
	if (WallrunData.Progress > WallrunSettings.MaxTime)
	{
		StopWallrunning(false);
	}
}

void UGCBaseCharacterMovementComponent::PhysCustomZiplining(float DeltaTime, int32 Iterations)
{
	const float g = -GetGravityZ();
	ZiplineParams.CurrentSpeed = ZiplineParams.CurrentSpeed + DeltaTime * g *
		(ZiplineParams.DeclinationAngleSin - ZiplineParams.Friction * ZiplineParams.DeclinationAngleCos);
	GEngine->AddOnScreenDebugMessage(3, 3, FColor::Green,
		FString::Printf(TEXT("Zipline unclamped speed: %f"), ZiplineParams.CurrentSpeed));
	Velocity = FMath::Clamp(ZiplineParams.CurrentSpeed, MinZiplineSpeed, MaxZiplineSpeed) * ZiplineParams.ZiplineNormalizedDirection;
	
	const FVector UncorrectedCharacterLocation = GetActorLocation();
	if (!UncorrectedCharacterLocation.Equals(ZiplineParams.CorrectedActorLocation, 2.f))
	{
		const float CorrectionRatio = 10.f;
		const FVector CorrectionDelta = (ZiplineParams.CorrectedActorLocation - UncorrectedCharacterLocation) * CorrectionRatio;
		Velocity += CorrectionDelta;
	}

	ZiplineParams.CorrectedActorLocation += FMath::Clamp(ZiplineParams.CurrentSpeed, 0.f, MaxZiplineSpeed)
		* ZiplineParams.ZiplineNormalizedDirection * DeltaTime;
	
	
	FVector Delta = Velocity * DeltaTime;
	FHitResult Hit;
	bool bMoved = SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
	if (!bMoved)
	{
		ZiplineObstacleHit.ExecuteIfBound(Hit);
	}
}

void UGCBaseCharacterMovementComponent::PhysCustomSliding(float DeltaTime, int32 Iterations)
{
	const float g = -GetGravityZ();
	FHitResult FloorCheckHit;
	FCollisionQueryParams FloorCheckCollisionQueryParams;
	FloorCheckCollisionQueryParams.AddIgnoredActor(CharacterOwner);
	const auto CapsuleShape = CharacterOwner->GetCapsuleComponent()->GetCollisionShape();
	const float TraceDepth = 3.f;
	const FVector FloorTraceStartLocation = GetActorLocation();
	bool bApplyGravity = !GetWorld()->SweepSingleByChannel(FloorCheckHit, FloorTraceStartLocation,
		FloorTraceStartLocation - CharacterOwner->GetActorUpVector() * TraceDepth,
		FQuat::Identity, ECC_Visibility, CapsuleShape, FloorCheckCollisionQueryParams);
	bool bDescending = true;
	if (!bApplyGravity)
	{
		SlideData.VerticalSpeed = 0.f;
		if (FloorCheckHit.ImpactNormal != FVector::ZeroVector)
		{
			FVector ProjectedActorForwardVector = CharacterOwner->GetActorForwardVector();
			ProjectedActorForwardVector.Z = 0.f;
			ProjectedActorForwardVector = ProjectedActorForwardVector.GetSafeNormal();
			const float SurfaceToForwardDP = FVector::DotProduct(FloorCheckHit.ImpactNormal, ProjectedActorForwardVector);
			bDescending = SurfaceToForwardDP >= 0.f;
			const float FloorAngle = FMath::Abs(90.f - FMath::RadiansToDegrees(FMath::Acos(SurfaceToForwardDP)));
			SlideData.FloorAngle = FloorAngle;
			SlideData.FloorAngleSin = FMath::Sin(FMath::DegreesToRadians(SlideData.FloorAngle));
			SlideData.FloorAngleCos = FMath::Cos(FMath::DegreesToRadians(SlideData.FloorAngle));
		}
	}
	else
	{
		SlideData.VerticalSpeed -= g * DeltaTime;
	}

	const float DirectionFactor = bDescending ? 1.f : -1.f;
	SlideData.Speed = SlideData.Speed
		+ DeltaTime * g * (DirectionFactor * SlideData.FloorAngleSin - SlideSettings.DeccelerationRate * SlideData.FloorAngleCos);
	
	if (SlideData.Speed < 0.f)
	{
		StopSliding();
		return;
	}

	FRotator CharacterRotation = CharacterOwner->GetActorRotation();
	CharacterRotation.Pitch = FMath::FInterpTo(CharacterRotation.Pitch, -DirectionFactor * SlideData.FloorAngle,
		DeltaTime, SlideSettings.PitchInterpolationSpeed);

	Velocity = FMath::Max(SlideData.Speed, 0.f) * CharacterRotation.Vector();
	if (bApplyGravity)
	{
		Velocity.Z = SlideData.VerticalSpeed;
	}
	
	FHitResult Hit;
	bool bHit = SafeMoveUpdatedComponent(Velocity * DeltaTime, CharacterRotation, true, Hit);
	if (bHit && Hit.bBlockingHit)
	{
		if (Hit.ImpactNormal.Z < GetWalkableFloorZ())
		{
			UE_LOG(LogTemp, Log, TEXT("Sliding blocking hit occured"));
			StopSliding();
		}
		else
		{
			// sometimes the character can just stuck on walkable surface so lifting him up a lil bit and moving without sweep
			SafeMoveUpdatedComponent((Velocity + FVector::UpVector * 2.5f) * DeltaTime, CharacterRotation, false, Hit);
		}
	}
}

bool UGCBaseCharacterMovementComponent::IsInCustomMovementMode(const EGCMovementMode& Mode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == (uint8)Mode;
}

void UGCBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForceTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForceTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent( FVector::ZeroVector, DesiredRotation,  false );
		}
		else
		{
			ForceTargetRotation = FRotator::ZeroRotator; 
			bForceRotation = false;
		}
		return;
	}
	
	if (IsClimbing() || IsWallrunning())
	{
		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

#pragma region UTILS

void UGCBaseCharacterMovementComponent::PreparePostureHalfHeights()
{
	const ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	PostureCapsuleHalfHeights.Add(EPosture::Standing, DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
	PostureCapsuleHalfHeights.Add(EPosture::Crouching, CrouchedHalfHeight);
	PostureCapsuleHalfHeights.Add(EPosture::Proning, ProneCapsuleHalfHeight);
	PostureCapsuleHalfHeights.ValueSort([] (const float& A, const float &B)
	{
		return A > B;
	});
}

UGCDebugSubsystem* UGCBaseCharacterMovementComponent::GetDebugSubsystem() const
{
	if (!IsValid(DebugSubsystem))
	{
		DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UGCDebugSubsystem>();
	}

	return DebugSubsystem;
}

#pragma endregion UTILS


