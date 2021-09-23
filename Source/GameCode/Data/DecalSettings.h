#pragma once

#include "DecalSettings.generated.h"

USTRUCT(BlueprintType)
struct FDecalSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UMaterialInterface* Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector Size = FVector(5.f,5.f,5.f);

	// in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float LifeTime = 10.f;

	// in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FadeOutTime = 5.f;
};