#pragma once

#include "CharacterTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterAttribute : uint8
{
	None,
	Health,
	Stamina,
	Oxygen,
};

UENUM(BlueprintType)
enum class ECharacterAction : uint8
{
	None,
	Move,
	Jump,
	Swim,
	Mantle,
	Zipline,
	Climb,
	Slide,
	Crouch,
	Crawl,
	HardLand,
	Sprint,
	Wallrun,
	
	Reload,
	ChangeEquipment,
	Shoot,
	Aim,
	MeleeAttack,
	ThrowItem,
	ToggleFireMode,
	
	Interact
};

USTRUCT(BlueprintType)
struct FCharacterActions
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSet<ECharacterAction> AffectedActions;
};