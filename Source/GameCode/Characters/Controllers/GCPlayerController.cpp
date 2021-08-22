// Fill out your copyright notice in the Description page of Project Settings.


#include "GCPlayerController.h"

#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"

void AGCPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	// checkf(InPawn->IsA<AGCBaseCharacter>(), TEXT("GCPlayerController::SetPawn: Pawn is not AGCBaseCharacter"));
	BaseCharacter = Cast<AGCBaseCharacter>(InPawn);
}

void AGCPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("Turn", this, &AGCPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AGCPlayerController::LookUp);
	InputComponent->BindAxis("TurnAtRate", this, &AGCPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpAtRate", this, &AGCPlayerController::LookUpAtRate);
	InputComponent->BindAxis("MoveForward", this, &AGCPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AGCPlayerController::MoveRight);

	InputComponent->BindAction("Mantle", EInputEvent::IE_Pressed, this, &AGCPlayerController::Mantle);
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AGCPlayerController::Jump);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &AGCPlayerController::StopSprint);
	
	InputComponent->BindAxis("SwimForward", this, &AGCPlayerController::SwimForward);
	InputComponent->BindAxis("SwimRight", this, &AGCPlayerController::SwimRight);
	InputComponent->BindAxis("SwimUp", this, &AGCPlayerController ::SwimUp);

	InputComponent->BindAxis("ClimbUp", this, &AGCPlayerController::ClimbUp);
	InputComponent->BindAxis("ClimbDown", this, &AGCPlayerController::ClimbDown);

	InputComponent->BindAction("Interact", IE_Pressed, this, &AGCPlayerController::Interact);

	InputComponent->BindAction("Slide", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartSliding);
	InputComponent->BindAction("Slide", EInputEvent::IE_Released, this, &AGCPlayerController::StopSliding);
	InputComponent->BindAction("Prone", IE_Pressed, this, &AGCPlayerController::ToggleProneState);
	InputComponent->BindAction("Crouch", IE_Pressed, this, &AGCPlayerController::ToggleCrouchState);

	InputComponent->BindAction("Wallrun", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartWallrun);
	InputComponent->BindAction("Wallrun", EInputEvent::IE_Released, this, &AGCPlayerController::StopWallrun);
	
}

void AGCPlayerController::Interact()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->Interact();
	}
}

void AGCPlayerController::ClimbDown(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->ClimbDown(Value);
	}
}

void AGCPlayerController::ClimbUp(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->ClimbUp(Value);
	}
}

void AGCPlayerController::SwimUp(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->SwimUp(Value);
	}
}

void AGCPlayerController::SwimRight(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->SwimRight(Value);
	}
}

void AGCPlayerController::SwimForward(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->SwimForward(Value);
	}
}

void AGCPlayerController::TurnAtRate(float Value)
{
	if (BaseCharacter.IsValid())
		BaseCharacter->TurnAtRate(Value);
}

void AGCPlayerController::LookUpAtRate(float Value)
{
	if (BaseCharacter.IsValid())
		BaseCharacter->LookUpAtRate(Value);
}

void AGCPlayerController::LookUp(float Value)
{
	if (BaseCharacter.IsValid())
		BaseCharacter->LookUp(Value);
}

void AGCPlayerController::MoveForward(float Value)
{
	if (BaseCharacter.IsValid())
		BaseCharacter->MoveForward(Value);
}

void AGCPlayerController::MoveRight(float Value)
{
	if (BaseCharacter.IsValid())
		BaseCharacter->MoveRight(Value);
}

void AGCPlayerController::Turn(float Value)
{
	if (BaseCharacter.IsValid())
		BaseCharacter->Turn(Value);
}

void AGCPlayerController::Mantle()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->Mantle();
	}
}

void AGCPlayerController::Jump()
{
	if (BaseCharacter.IsValid())
		BaseCharacter->Jump();
}

void AGCPlayerController::StartSliding()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->TryStartSliding();
	}
}

void AGCPlayerController::StopSliding()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->StopSliding();
	}
}


void AGCPlayerController::ToggleCrouchState()
{
	if (BaseCharacter.IsValid())
		BaseCharacter->ToggleCrouchState();
}

void AGCPlayerController::ToggleProneState()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->ToggleProneState();
	}
}

void AGCPlayerController::StartSprint()
{
	if (BaseCharacter.IsValid())
		BaseCharacter->StartSprint();
}

void AGCPlayerController::StopSprint()
{
	if (BaseCharacter.IsValid())
		BaseCharacter->StopSprint();
}

void AGCPlayerController::StartWallrun()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->StartWallrun();
	}
}

void AGCPlayerController::StopWallrun()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->StopWallrun();
	}
}