// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Controllers/GCPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

AFPPlayerCharacter::AFPPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FirstPersonMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMeshComponent->SetupAttachment(RootComponent);
	FirstPersonMeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -86.f));
	FirstPersonMeshComponent->CastShadow = false;
	FirstPersonMeshComponent->bCastDynamicShadow = false;
	FirstPersonMeshComponent->SetOnlyOwnerSee(true);
	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMeshComponent, SocketFPCamera); // GetCapsuleComponent?
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;

	CameraComponent->SetAutoActivate(false);
	SpringArmComponent->SetAutoActivate(false);
	SpringArmComponent->bUsePawnControlRotation = false;

	GCMovementComponent->bOrientRotationToMovement = false;

	bUseControllerRotationYaw = true;
}

void AFPPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	OnMontageEnded.BindUObject(this, &AFPPlayerCharacter::OnMontageEndedCallback);

}

void AFPPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (IsFPMontagePlaying() && GCPlayerController.IsValid())
	{
		FRotator TargetControlRotation = GCPlayerController->GetControlRotation();
		TargetControlRotation.Pitch = 0.f;
		const float BlendSpeed = 50.f;
		TargetControlRotation = FMath::RInterpTo(GCPlayerController->GetControlRotation(), TargetControlRotation,
			DeltaSeconds, BlendSpeed);
		GCPlayerController->SetControlRotation(TargetControlRotation);
	}
}

void AFPPlayerCharacter::OnStartCrouchOrProne(float HalfHeightAdjust)
{
	Super::OnStartCrouchOrProne(HalfHeightAdjust);
	const auto DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
    FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
    FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z + HalfHeightAdjust;
}

void AFPPlayerCharacter::OnEndCrouchOrProne(float HalfHeightAdjust)
{
	Super::OnEndCrouchOrProne(HalfHeightAdjust);
	const auto DefaultCharacter = GetDefault<AFPPlayerCharacter>(GetClass());
	FVector& FirstPersonMeshRelativeLocation = FirstPersonMeshComponent->GetRelativeLocation_DirectMutable();
	FirstPersonMeshRelativeLocation.Z = DefaultCharacter->FirstPersonMeshComponent->GetRelativeLocation().Z;
}

void AFPPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	GCPlayerController = Cast<AGCPlayerController>(NewController);
}

bool AFPPlayerCharacter::IsFPMontagePlaying() const
{
	const auto AnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	return IsValid(AnimInstance) && AnimInstance->IsAnyMontagePlaying();
}

FRotator AFPPlayerCharacter::GetViewRotation() const
{
	FRotator Result = Super::GetViewRotation();
	if (IsFPMontagePlaying())
	{
		FRotator SocketRotation = FirstPersonMeshComponent->GetSocketRotation(SocketFPCamera);
		Result.Yaw = SocketRotation.Yaw;
		Result.Roll = SocketRotation.Roll;
		Result.Pitch += SocketRotation.Pitch;
	}

	return Result;
}

void AFPPlayerCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	if (!GCPlayerController.IsValid())
	{
		return;
	}
	
	if (GCMovementComponent->IsClimbing())
	{
		const FRotator ActorRotation = GetActorRotation();
		const float YawAdjustment = ActorRotation.Yaw;
		RestrictCameraMovement(ClimbingCameraPitchMin, ClimbingCameraPitchMax,
			ClimbingCameraYawMin, ClimbingCameraYawMax, YawAdjustment);
	}
	else if (GCMovementComponent->IsZiplining())
	{
		const FRotator ActorRotation = GCMovementComponent->GetZiplineParams().ZiplineNormalizedDirection.ToOrientationRotator();
		const float YawAdjustment = ActorRotation.Yaw;
		RestrictCameraMovement(ClimbingCameraPitchMin, ClimbingCameraPitchMax,
			ClimbingCameraYawMin, ClimbingCameraYawMax, YawAdjustment);
	}
	else
	{
		if (PrevMovementMode == EMovementMode::MOVE_Custom)
		{
			switch ((EGCMovementMode)PreviousCustomMode)
			{
				case EGCMovementMode::CMOVE_Climbing:
				case EGCMovementMode::CMOVE_Zipline:
					{
						GCPlayerController->SetIgnoreCameraPitch(false);
						bUseControllerRotationYaw = true;
						const auto DefaultCameraManager = GCPlayerController->PlayerCameraManager->GetClass()->GetDefaultObject<APlayerCameraManager>();
						GCPlayerController->PlayerCameraManager->ViewPitchMin = DefaultCameraManager->ViewPitchMin;
						GCPlayerController->PlayerCameraManager->ViewPitchMax = DefaultCameraManager->ViewPitchMax;
						GCPlayerController->PlayerCameraManager->ViewYawMin = DefaultCameraManager->ViewYawMin;
						GCPlayerController->PlayerCameraManager->ViewYawMax = DefaultCameraManager->ViewYawMax;
						break;
					}
				default:
					break;
			}
		}
	}
}

void AFPPlayerCharacter::RestrictCameraMovement(float PitchMin, float PitchMax, float YawMin, float YawMax,
	float YawAdjustment)
{
	GCPlayerController->SetIgnoreCameraPitch(true);
	bUseControllerRotationYaw = false;
	GCPlayerController->PlayerCameraManager->ViewPitchMin = PitchMin;
	GCPlayerController->PlayerCameraManager->ViewPitchMax = PitchMax;
	GCPlayerController->PlayerCameraManager->ViewYawMin = YawMin + YawAdjustment; 
	GCPlayerController->PlayerCameraManager->ViewYawMax = YawMax + YawAdjustment;
}


void AFPPlayerCharacter::PlayMantleMontage(const FMantlingSettings& MantleSettings, float StartTime)
{
	Super::PlayMantleMontage(MantleSettings, StartTime);
	UAnimInstance* AnimInstance = FirstPersonMeshComponent->GetAnimInstance();
	if (IsValid(AnimInstance) && IsValid(MantleSettings.MantleMontageFP))
	{
		SetInputDisabled(true, true);
		AnimInstance->Montage_Play(MantleSettings.MantleMontageFP, 1, EMontagePlayReturnType::Duration, StartTime);
		AnimInstance->Montage_SetEndDelegate(OnMontageEnded, MantleSettings.MantleMontageFP);
	}
}

FMontagePlayResult AFPPlayerCharacter::PlayHardLandMontage()
{
	Super::PlayHardLandMontage();
	FMontagePlayResult Result = Super::PlayHardLandMontage(FirstPersonMeshComponent->GetAnimInstance(), HardLandMontageFP);
	Result.bDisableCameraRotation = true;
	return Result;
}

void AFPPlayerCharacter::OnMontageEndedCallback(UAnimMontage* AnimMontage, bool bInterrupted)
{
	SetInputDisabled(false, false);
}
