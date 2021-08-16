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

	SprintSpringArmTimelineCurve = CreateDefaultSubobject<UCurveFloat>(TEXT("Sprint SpringArm Curve"));
	// TODO single curve for sprint and wallrun offset
	const auto& timelineStart = SprintSpringArmTimelineCurve->FloatCurve.AddKey(0,0);
	const auto& timelineEnd = SprintSpringArmTimelineCurve->FloatCurve.AddKey(SprintSpringArmOffsetDuration, 1);
	SprintSpringArmTimelineCurve->FloatCurve.SetKeyInterpMode(timelineStart, ERichCurveInterpMode::RCIM_Cubic);
	SprintSpringArmTimelineCurve->FloatCurve.SetKeyInterpMode(timelineEnd, ERichCurveInterpMode::RCIM_Cubic);
	SprintSpringArmTimelineCurve->FloatCurve.BakeCurve(60.f);

	DefaultSpringArmOffset = SpringArmComponent->TargetArmLength;
}

void APlayerCharacter::InitSprintSpringArm()
{
	if (IsValid(SprintSpringArmTimelineCurve))
	{
		FOnTimelineFloatStatic SprintSpringArmTimelineCallback;
		SprintSpringArmTimelineCallback.BindUObject(this, &APlayerCharacter::SetSpringArmPosition);
		SprintSpringArmTimeline.AddInterpFloat(SprintSpringArmTimelineCurve, SprintSpringArmTimelineCallback);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitSprintSpringArm();
	GCMovementComponent->WallrunBeginEvent.BindUObject(this, &APlayerCharacter::OnWallrunBegin);
	GCMovementComponent->WallrunEndEvent.BindUObject(this, &APlayerCharacter::OnWallrunEnd);
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (SprintSpringArmTimeline.IsPlaying())
	{
		SprintSpringArmTimeline.TickTimeline(DeltaSeconds);		
	}
}

void APlayerCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);	
}

void APlayerCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);	
}

void APlayerCharacter::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APlayerCharacter::TurnAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds() );
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
		UnCrouch();
}

void APlayerCharacter::SetSpringArmPosition(float alpha) const
{
	SpringArmComponent->TargetArmLength = FMath::Lerp(DefaultSpringArmOffset, SprintSpringArmOffset, alpha);
}

void APlayerCharacter::OnSprintStart_Implementation()
{
	SprintSpringArmTimeline.Play();
}

void APlayerCharacter::OnSprintEnd_Implementation()
{
	SprintSpringArmTimeline.Reverse();	
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
