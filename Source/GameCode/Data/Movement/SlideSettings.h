#pragma once

#include "SlideSettings.generated.h"

USTRUCT(BlueprintType)
struct FSlideSettings
{
	GENERATED_BODY()

	// Max slide duration in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.5", UIMin="0.5"))
	float Duration = 2.0f;

	// Rate at which character slows down when sliding. Surface friction is not considered (yet)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0", UIMin="0.0"))
	float DeccelerationRate = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="30", UIMin="30"))
	float CapsuleHalfHeight = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="200", UIMin="200"))
	float MaxSpeed = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PitchInterpolationSpeed = 75.f;

	// Time between next slide attempt after previous finished
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0", UIMin="200"))
	float CooldownTime = 0.5f;
};
