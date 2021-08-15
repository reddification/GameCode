#pragma once

#include "IKSettings.generated.h"

USTRUCT(BlueprintType)
struct FIKSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK")
	FName RightFootSocketName = "RightFootSocketIK";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK")
	FName LeftFootSocketName = "LeftFootSocketIK";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK")
	FName RightHeelSocketName = "RightHeelSocket";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK")
	FName RightToesSocketName = "RightToesSocket";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK")
	FName LeftHeelSocketName = "LeftHeelSocket";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK")
	FName LeftToesSocketName = "LeftToesSocket";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float IKInterpSpeed = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float IKTraceDistance = 60;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float IKTraceDistanceCrouch = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float TraceExtend = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float FootLength = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float FootWidth = 7;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float MinFootPitchForElevationTrace = 35;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK", meta=(ClampMin = 0, UIMin = 0))
	float MaxKneeOutwardExtend = 40;
};
