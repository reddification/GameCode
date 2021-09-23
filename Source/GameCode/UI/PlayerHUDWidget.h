// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"


UCLASS(BlueprintType)
class GAMECODE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealth(float Value);

	void OnAimingStateChanged(bool bAiming);
	void SetAmmo(int32 ClipAmmo, int32 RemainingAmmo);

protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UProgressBar* Healthbar;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UReticleWidget* Reticle;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UAmmoWidget* AmmoWidget;
};
