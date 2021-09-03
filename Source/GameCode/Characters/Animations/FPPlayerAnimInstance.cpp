// Fill out your copyright notice in the Description page of Project Settings.


#include "FPPlayerAnimInstance.h"

#include "GameCode/Characters/FPPlayerCharacter.h"

void UFPPlayerAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	const auto Pawn = TryGetPawnOwner();
	checkf(Pawn->IsA<AFPPlayerCharacter>(), TEXT("UFPPlayerAnimInstance is only for AFPPlayerCharacter"));
	FPPlayer = StaticCast<AFPPlayerCharacter*>(Pawn);
	GCPlayerController = FPPlayer->GetController<AGCPlayerController>();
}

void UFPPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!FPPlayer.IsValid())
	{
		return;
	}

	PlayerCameraPitch = GetCameraPitchAngle();
}

float UFPPlayerAnimInstance::GetCameraPitchAngle() const
{
	float Result = 0.f;
	if (GCPlayerController.IsValid() && !GCPlayerController->IsIgnoreCameraPitch())
	{
		Result = -GCPlayerController->GetControlRotation().Pitch;
	}

	return Result;
}
