// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MantlingSettings.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantleMontageTP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantleMontageFP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* MantleCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=0.0f, UIMin = 0.0f))
	float MaxHeight = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=0.0f, UIMin = 0.0f))
	float MinHeight = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=0.0f, UIMin = 0.0f))
	float MaxHeightStartTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=0.0f, UIMin = 0.0f))
	float MinHeightStartTime = 0.5f;

};
