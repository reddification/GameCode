#pragma once

#include "CoreMinimal.h"
#include "BarrelComponent.h"
#include "Data/FireModeSettings.h"
#include "WeaponBarrelComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UWeaponBarrelComponent : public UBarrelComponent
{
	GENERATED_BODY()

public:
	const FFireModeSettings& GetFireModeSettings() const { return FireModeSettings; }
	
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FFireModeSettings FireModeSettings;
};
