#pragma once

UENUM(BlueprintType)
enum class ESide : uint8
{
	None UMETA(DisplayName="None"),
	Left UMETA(DisplayName="Left"),
	Right UMETA(DisplayName="Right"),
	Front UMETA(DisplayName="Front")
};