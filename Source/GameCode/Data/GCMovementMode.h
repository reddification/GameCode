#pragma once

UENUM(BlueprintType)
enum class EGCMovementMode : uint8
{
	CMOVE_None UMETA(DisplayName="None"),
	CMOVE_Mantling UMETA(DisplayName="Mantling"),
	CMOVE_Climbing UMETA(DisplayName="Climbing"),
	CMOVE_WallRun UMETA(DisplayName = “Wall run”),
	CMOVE_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESide : uint8
{
	None UMETA(DisplayName="None"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
	Front UMETA(DisplayName="Front")
};