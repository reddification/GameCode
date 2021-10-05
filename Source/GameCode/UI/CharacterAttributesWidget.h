// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CharacterTypes.h"
#include "CharacterAttributesWidget.generated.h"

class UProgressBar;
UCLASS()
class GAMECODE_API UCharacterAttributesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAttribute(ECharacterAttribute Attribute, float Value);
	
protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* StaminaBar;

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UProgressBar* OxygenBar;

	UFUNCTION(BlueprintImplementableEvent)
	void OnAttributeRestored(ECharacterAttribute CharacterAttribute);

	UFUNCTION(BlueprintImplementableEvent)
	void OnAttributeChanged(ECharacterAttribute CharacterAttribute);
};
