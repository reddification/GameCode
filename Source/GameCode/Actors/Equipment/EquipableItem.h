#pragma once

#include "CoreMinimal.h"
#include "Data/EquipmentTypes.h"
#include "Data/UserInterfaceTypes.h"
#include "GameFramework/Actor.h"
#include "EquipableItem.generated.h"

class UAnimMontage;
class UNameplateComponent;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AEquipableItem : public AActor
{
	GENERATED_BODY()

public:
	AEquipableItem();
	
	EEquippableItemType GetEquippableItemType() const { return EquippableItemType; }

	FName GetCharacterEquippedSocketName() const { return CharacterEquippedSocketName; }
	FName GetCharacterUnequippedSocketName() const { return CharacterUnequippedSocketName; }

	UAnimMontage* GetCharacterEquipMontage() const { return CharacterEquipMontage; }
	float GetEquipmentDuration() const { return EquipmentDuration; }

	const UNameplateComponent* GetNameplateComponent() const { return NameplateComponent; }

	virtual EReticleType GetReticleType() const { return ReticleType; }
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UNameplateComponent* NameplateComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReticleType ReticleType = EReticleType::None;
};
