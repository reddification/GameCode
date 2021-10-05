// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterAttributesWidget.h"
#include "Blueprint/UserWidget.h"
#include "Data/CharacterTypes.h"
#include "Data/UserInterfaceTypes.h"
#include "PlayerHUDWidget.generated.h"


UCLASS(BlueprintType)
class GAMECODE_API UPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void OnAimingStateChanged(bool bAiming, EReticleType ReticleType);
	void SetAmmo(int32 ClipAmmo, int32 RemainingAmmo);
	void OnAttributeChanged(ECharacterAttribute Attribute, float Value);
	void SetReticleType(EReticleType ReticleType);

protected:

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UReticleWidget* Reticle;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UAmmoWidget* AmmoWidget;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UCharacterAttributesWidget* CharacterAttributesWidget;
};
