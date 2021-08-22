#pragma once

UENUM(BlueprintType)
enum class EGCMovementMode : uint8
{
	CMOVE_None UMETA(DisplayName="None"),
	CMOVE_Mantling UMETA(DisplayName="Mantling"),
	CMOVE_Climbing UMETA(DisplayName="Climbing"),
	CMOVE_Zipline UMETA(DisplayName="Zipline"),
	CMOVE_Slide UMETA(DisplayName="Sliding"),
	CMOVE_WallRun UMETA(DisplayName = “Wall run”),
	CMOVE_Max UMETA(Hidden)
};