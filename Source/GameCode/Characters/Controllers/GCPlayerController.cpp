// Fill out your copyright notice in the Description page of Project Settings.


#include "GCPlayerController.h"

#include "Components/CharacterAttributesComponent.h"
#include "Components/CharacterEquipmentComponent.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Components/NameplateComponent.h"
#include "GameCode/Characters/GCBaseCharacter.h"
#include "GameCode/Components/Movement/GCBaseCharacterMovementComponent.h"

void AGCPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	BaseCharacter = Cast<AGCBaseCharacter>(InPawn);
	if (BaseCharacter.IsValid())
	{
		UCharacterEquipmentComponent* EquipmentComponent = BaseCharacter->GetEquipmentComponent();
		BaseCharacter->AimingStateChangedEvent.BindUObject(this, &AGCPlayerController::OnAimingStateChanged);
		BaseCharacter->GetCharacterAttributesComponent()->AttributeChangedEvent.AddUObject(this, &AGCPlayerController::OnAttributeChanged);
		
		EquipmentComponent->WeaponAmmoChangedEvent.BindUObject(this, &AGCPlayerController::OnAmmoChanged);
		EquipmentComponent->WeaponEquippedChangedEvent.AddUObject(this, &AGCPlayerController::OnWeaponEquipStateChanged);
	}
}

void AGCPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AGCPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(PlayerHUDWidgetClass))
	{
		PlayerHUDWidget = CreateWidget<UPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		PlayerHUDWidget->AddToViewport();
	}
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

	InputComponent->BindAction("Fire", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartFiring);
	InputComponent->BindAction("Fire", EInputEvent::IE_Released, this, &AGCPlayerController::StopFiring);

	InputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &AGCPlayerController::StartAiming);
	InputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &AGCPlayerController::StopAiming);

	InputComponent->BindAction("Reload", EInputEvent::IE_Pressed, this, &AGCPlayerController::Reload);

	InputComponent->BindAction("NextWeapon", EInputEvent::IE_Pressed, this, &AGCPlayerController::PickNextWeapon);
	InputComponent->BindAction("PreviousWeapon", EInputEvent::IE_Pressed, this, &AGCPlayerController::PickPreviousWeapon);

	InputComponent->BindAction("ThrowItem", EInputEvent::IE_Pressed, this, &AGCPlayerController::ThrowItem);
}

void AGCPlayerController::Interact() 
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->Interact();
	}
}

void AGCPlayerController::ClimbDown(float Value) 
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->ClimbDown(Value);
	}
}

void AGCPlayerController::ClimbUp(float Value) 
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->ClimbUp(Value);
	}
}

void AGCPlayerController::SwimUp(float Value) 
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->SwimUp(Value);
	}
}

void AGCPlayerController::SwimRight(float Value)
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->SwimRight(Value);
	}
}

void AGCPlayerController::SwimForward(float Value)
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->SwimForward(Value);
	}
}

void AGCPlayerController::TurnAtRate(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->TurnAtRate(Value);
	}
}

void AGCPlayerController::LookUpAtRate(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->LookUpAtRate(Value);
	}
}

void AGCPlayerController::LookUp(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->LookUp(Value);
	}
}

void AGCPlayerController::MoveForward(float Value)
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->MoveForward(Value);
	}
}

void AGCPlayerController::MoveRight(float Value)
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->MoveRight(Value);
	}
}

void AGCPlayerController::Turn(float Value)
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->Turn(Value);
	}
}

void AGCPlayerController::Mantle()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->Mantle();
	}
}

void AGCPlayerController::Jump()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->Jump();
	}
}

void AGCPlayerController::StartSliding()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
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
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->ToggleCrouchState();
	}
}

void AGCPlayerController::ToggleProneState()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->ToggleProneState();
	}
}

void AGCPlayerController::StartSprint()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->StartRequestingSprint();
	}
}

void AGCPlayerController::StopSprint()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->StopRequestingSprint();
	}
}

void AGCPlayerController::StartWallrun()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->StartRequestingWallrun();
	}
}

void AGCPlayerController::StopWallrun()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->StopRequestingWallrun();
	}
}

void AGCPlayerController::StartFiring()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->StartFiring();		
	}
}

void AGCPlayerController::StartAiming()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->StartAiming();		
	}
}

void AGCPlayerController::StopAiming()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->StopAiming();		
	}
}

void AGCPlayerController::Reload()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->StartReloading();
	}
}

void AGCPlayerController::PickNextWeapon()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->PickWeapon(1);
	}
}

void AGCPlayerController::PickPreviousWeapon()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->PickWeapon(-1);
	}
}

void AGCPlayerController::OnAttributeChanged(ECharacterAttribute Attribute, float Percent) const
{
	if (IsValid(PlayerHUDWidget))
	{
		PlayerHUDWidget->OnAttributeChanged(Attribute, Percent);
	}
}

void AGCPlayerController::ThrowItem()
{
	if (BaseCharacter.IsValid() && BaseCharacter->IsMovementInputEnabled())
	{
		BaseCharacter->ThrowItem();
	}
}

void AGCPlayerController::StopFiring()
{
	if (BaseCharacter.IsValid())
	{
		BaseCharacter->StopFiring();
	}
}

void AGCPlayerController::OnAimingStateChanged(bool bAiming, EReticleType NewReticleType, ARangeWeaponItem* Weapon)
{
	if (IsValid(PlayerHUDWidget))
	{
		PlayerHUDWidget->OnAimingStateChanged(bAiming, NewReticleType);
	}

	if (IsValid(Weapon) && Weapon->GetAimReticleType() == EReticleType::Scope)
	{
		FViewTargetTransitionParams TransitionParams;
		TransitionParams.BlendTime = 0.2;
		if (bAiming)
		{
			SetViewTarget(Weapon, TransitionParams);
		}
		else
		{
			SetViewTarget(BaseCharacter.Get(), TransitionParams);
		}
	}
}

void AGCPlayerController::OnAmmoChanged(int32 NewAmmo, int32 TotalAmmo)
{
	if (IsValid(PlayerHUDWidget))
	{
		PlayerHUDWidget->SetAmmo(NewAmmo, TotalAmmo);
	}
}

void AGCPlayerController::OnWeaponEquipStateChanged(ARangeWeaponItem* EquippedWeapon, bool bEquipped)
{
	if (IsValid(PlayerHUDWidget))
	{
		PlayerHUDWidget->SetReticleType(bEquipped ? EquippedWeapon->GetReticleType() : EReticleType::None);
		// PlayerHUDWidget->SetWeaponName(EquippedWeapon->GetNameplateComponent()->GetName());
	}
	
}