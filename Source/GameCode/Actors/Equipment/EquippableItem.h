#pragma once

#include "CoreMinimal.h"
#include "Components/NameplateComponent.h"
#include "Data/EquipmentTypes.h"
#include "Data/UserInterfaceTypes.h"
#include "GameFramework/Actor.h"
#include "EquippableItem.generated.h"

class UAnimMontage;
class UNameplateComponent;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AEquippableItem : public AActor
{
	GENERATED_BODY()

public:
	AEquippableItem();
	
	EEquippableItemType GetEquippableItemType() const { return EquippableItemType; }

	FName GetCharacterEquippedSocketName() const { return CharacterEquippedSocketName; }
	FName GetCharacterUnequippedSocketName() const { return CharacterUnequippedSocketName; }

	UAnimMontage* GetCharacterEquipMontage() const { return CharacterEquipMontage; }
	float GetEquipmentDuration() const { return EquipmentDuration; }

	const UNameplateComponent* GetNameplateComponent() const { return NameplateComponent; }

	virtual EReticleType GetReticleType() const { return ReticleType; }

	// TODO OOP :(
	bool IsRangedWeapon() const;
	bool IsMeleeWeapon() const;
	bool IsThrowable() const;

	const FText& GetName() { return NameplateComponent->GetName(); }

	void OnDropped();
	
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UNameplateComponent* NameplateComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EReticleType ReticleType = EReticleType::None;
};
