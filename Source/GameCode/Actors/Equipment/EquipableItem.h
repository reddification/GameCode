// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/EquippableItemType.h"
#include "GameFramework/Actor.h"
#include "EquipableItem.generated.h"

class UAnimMontage;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AEquipableItem : public AActor
{
	GENERATED_BODY()

public:
	EEquippableItemType GetEquippableItemType() const { return EquippableItemType; }

	FName GetCharacterEquippedSocketName() const { return CharacterEquippedSocketName; }
	FName GetCharacterUnequippedSocketName() const { return CharacterUnequippedSocketName; }

	UAnimMontage* GetCharacterEquipMontage() const { return CharacterEquipMontage; }
	float GetEquipmentDuration() const { return EquipmentDuration; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EEquippableItemType EquippableItemType = EEquippableItemType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CharacterEquippedSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CharacterUnequippedSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* CharacterEquipMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float EquipmentDuration = 1.f;
};
