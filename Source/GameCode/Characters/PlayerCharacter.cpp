// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameCode/Actors/Interactive/Environment/Ladder.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = 1;
	GetCharacterMovement()->JumpZVelocity = 600.f;

	DefaultSpringArmOffset = SpringArmComponent->TargetArmLength;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	DefaultFOV = CameraComponent->FieldOfView;
	InitTimeline(SprintSpringArmTimeline, SprintSpringArmTimelineCurve, &APlayerCharacter::SetSpringArmPosition);
	InitTimeline(AimFovAdjustmentTimeline, AimFovAdjustmentCurve, &APlayerCharacter::SetAimFovPosition);
	GCMovementComponent->WallrunBeginEvent.AddUObject(this, &APlayerCharacter::OnWallrunBegin);
	GCMovementComponent->WallrunEndEvent.AddUObject(this, &APlayerCharacter::OnWallrunEnd);
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	for (FTimeline* ActiveTimeline : ActiveTimelines)
	{
		if (ActiveTimeline->IsPlaying() || ActiveTimeline->IsReversing())
		{
			ActiveTimeline->TickTimeline(DeltaSeconds);
		}
	}
}

void APlayerCharacter::InitTimeline(FTimeline& Timeline, UCurveFloat* Curve, void(APlayerCharacter::* Callback)(float) const)
{
	if (IsValid(Curve))
	{
		FOnTimelineFloatStatic TimelineCallback;
		TimelineCallback.BindUObject(this, Callback);
		Timeline.AddInterpFloat(Curve, TimelineCallback);
		ActiveTimelines.Add(&Timeline);
	}
}

void APlayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value * LookUpModifier);	
}

void APlayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value * TurnModifier);	
}

void APlayerCharacter::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpRate * LookUpModifier * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::TurnAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseTurnRate * TurnModifier * GetWorld()->GetDeltaSeconds() );
}

void APlayerCharacter::MoveForward(float Value)
{
	Super::MoveForward(Value);
	if (!GetMovementComponent()->IsFalling() && !GetMovementComponent()->IsMovingOnGround())
	{
		return;
	}
	
	if (!FMath::IsNearlyZero(Value, SMALL_NUMBER))
	{
		const FRotator Rotator(0, GetControlRotation().Yaw, 0);
		FVector ForwardVector = Rotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);	
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	Super::MoveRight(Value);
	if (!GetMovementComponent()->IsFalling() && !GetMovementComponent()->IsMovingOnGround())
	{
		return;
	}
	
	if (!FMath::IsNearlyZero(Value, SMALL_NUMBER))
	{
		const FRotator Rotator(0, GetControlRotation().Yaw, 0);
		FVector RightVector = Rotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);	
	}
}

void APlayerCharacter::SwimForward(float Value)
{
	if (!GetMovementComponent()->IsSwimming())
	{
		return;
	}
	
	Super::SwimForward(Value);
	if (!FMath::IsNearlyZero(Value, SMALL_NUMBER))
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator Rotator(ControlRotation.Pitch, ControlRotation.Yaw, 0);
		FVector ForwardVector = Rotator.RotateVector(FVector::ForwardVector);
		AddMovementInput(ForwardVector, Value);	
	}
}

void APlayerCharacter::SwimRight(float Value)
{
	if (!GetMovementComponent()->IsSwimming())
	{
		return;
	}
	
	Super::SwimRight(Value);
	if (!FMath::IsNearlyZero(Value, SMALL_NUMBER))
	{
		const FRotator YawRotator(0, GetControlRotation().Yaw, 0);
		FVector RightVector = YawRotator.RotateVector(FVector::RightVector);
		AddMovementInput(RightVector, Value);	
	}
}

void APlayerCharacter::SwimUp(float Value)
{
	if (GetMovementComponent()->IsSwimming() && !FMath::IsNearlyZero(Value, SMALL_NUMBER))
	{
		FVector Direction;
		if (Value > 0)
		{
			Direction = GetActorForwardVector();
			Direction.Z = 0.99f;	
		}
		else
		{
			Direction = FVector::UpVector;
		}
		
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::ClimbUp(float Value)
{
	if (GetGCMovementComponent()->IsClimbing() && !FMath::IsNearlyZero(Value))
	{
		const auto Climbable = GetGCMovementComponent()->GetCurrentClimbable();
		FVector ClimbingUpVector(Climbable->GetActorUpVector());
		AddMovementInput(ClimbingUpVector, Value);
	}
}

void APlayerCharacter::ClimbDown(float Value)
{
	if (GetGCMovementComponent()->IsClimbing() && !FMath::IsNearlyZero(Value))
	{
		FVector ClimbingDownVector(-GetGCMovementComponent()->GetCurrentClimbable()->GetActorUpVector());
		AddMovementInput(ClimbingDownVector, Value);
	}
} 

void APlayerCharacter::AdjustSpringArm(const FVector& Adjustment)
{
	SpringArmComponent->TargetOffset += Adjustment;
}

void APlayerCharacter::AdjustSpringArmRelative(const FVector& Adjustment)
{
	SpringArmComponent->AddRelativeLocation(Adjustment);
}


void APlayerCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void APlayerCharacter::OnSprintStart_Implementation()
{
	if (IsValid(SprintSpringArmTimelineCurve))
	{
		SprintSpringArmTimeline.Play();
	}
}

void APlayerCharacter::OnSprintEnd_Implementation()
{
	if (IsValid(SprintSpringArmTimelineCurve))
	{
		SprintSpringArmTimeline.Reverse();
	}
}

void APlayerCharacter::OnStartCrouchOrProne(float HalfHeightAdjust)
{
	Super::OnStartCrouchOrProne(HalfHeightAdjust);
	AdjustSpringArm(FVector(0, 0, HalfHeightAdjust));
}

void APlayerCharacter::OnEndCrouchOrProne(float HalfHeightAdjust)
{
	Super::OnEndCrouchOrProne(HalfHeightAdjust);
	AdjustSpringArm(FVector(0, 0, -HalfHeightAdjust));
}

void APlayerCharacter::OnWallrunBegin(ESide Side)
{
	OnWallrunChanged(Side, 1);
}

void APlayerCharacter::OnWallrunEnd(ESide Side)
{
	OnWallrunChanged(Side, -1);
}

void APlayerCharacter::OnWallrunChanged(ESide Side, int AdjustmentModification)
{
	const float CameraPositionAdjustment = 50.f;
	switch (Side)
	{
	case ESide::Left:
		break;
	case ESide::Right:
		AdjustmentModification *= -1;
		break;
	default:
		break;
	}

	// TODO timeline curve
	AdjustSpringArmRelative(FVector(0, AdjustmentModification * CameraPositionAdjustment, 0));
}

void APlayerCharacter::OnSlidingStateChangedEvent(bool bSliding, float HalfHeightAdjust)
{
	Super::OnSlidingStateChangedEvent(bSliding, HalfHeightAdjust);
	AdjustSpringArm(FVector(0, 0, bSliding ? HalfHeightAdjust : -HalfHeightAdjust));
}

void APlayerCharacter::OnAimingStart_Implementation(float FOV, float NewTurnModifier, float NewLookUpModifier)
{
	Super::OnAimingStart_Implementation(FOV, NewTurnModifier, NewLookUpModifier);
	TurnModifier = NewTurnModifier;
	LookUpModifier = NewLookUpModifier;
	if (IsValid(AimFovAdjustmentCurve))
	{
		AimFovAdjustmentTimeline.Play();
		AimingFOV = FOV;
	}
	else
	{
		CameraComponent->SetFieldOfView(FOV);
	}
}

void APlayerCharacter::OnAimingEnd_Implementation()
{
	Super::OnAimingEnd_Implementation();
	TurnModifier = 1.f;
	LookUpModifier = 1.f;
	if (IsValid(AimFovAdjustmentCurve))
	{
		AimFovAdjustmentTimeline.Reverse();
	}
	else
	{
		CameraComponent->SetFieldOfView(DefaultFOV);
	}
}

void APlayerCharacter::SetSpringArmPosition(float Alpha) const
{
	SpringArmComponent->TargetArmLength = FMath::Lerp(DefaultSpringArmOffset, SprintSpringArmOffset, Alpha);
}

void APlayerCharacter::SetAimFovPosition(float Alpha) const
{
	CameraComponent->SetFieldOfView(FMath::Lerp(DefaultFOV, AimingFOV, Alpha));
}
