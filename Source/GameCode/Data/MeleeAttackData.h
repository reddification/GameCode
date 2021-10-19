#pragma once

#include "MeleeAttackData.generated.h"

USTRUCT(BlueprintType)
struct FMeleeAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, UIMin=0.f))
	float DamageAmount = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.1f, UIMin=0.1f))
	float DesiredDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bChargeable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.0f, UIMin=0.0f))
	float StaminaConsumption = 10.f;
};