// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/PlayerController.h"
#include "UI/PlayerHUDWidget.h"
#include "GCPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API AGCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	bool IsIgnoreCameraPitch() const { return bIgnoreCameraPitch; }
	void SetIgnoreCameraPitch(bool newValue) { bIgnoreCameraPitch = newValue; }

	virtual void BeginPlay() override;
	
protected:
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UPlayerHUDWidget* PlayerHUDWidget;
	
private:
	TSoftObjectPtr<class AGCBaseCharacter> BaseCharacter;
	
	void StartSliding();
	void StopSliding();
	void StartWallrun();
	void StopWallrun();
	void Interact();
	void ClimbDown(float Value);
	void ClimbUp(float Value);
	void SwimUp(float Value);
	void SwimRight(float Value);
	void SwimForward(float Value);

	void TurnAtRate(float Value);
	void LookUpAtRate(float Value);
	void LookUp (float Value);
	void Turn(float Value);
	void MoveForward (float Value);
	void MoveRight (float Value);

	void Mantle();
	void Jump();
	void ToggleCrouchState();
	void ToggleProneState();

	void StartSprint();
	void StopSprint();

	void StopFiring();
	void StartFiring();
	void StartAiming();
	void StopAiming();
	void Reload();

	void PickNextWeapon();
	void PickPreviousWeapon();
	
	void OnHealthChanged(float HealthPercent) const;
	
	bool bIgnoreCameraPitch;
	void OnAimingStateChanged(bool bAiming);
	void OnAmmoChanged(int32 NewAmmo, int32 TotalAmmo);

};
