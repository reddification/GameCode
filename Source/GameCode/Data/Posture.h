#pragma once

UENUM(BlueprintType)
enum class EPosture : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Standing = 1 UMETA(DisplayName="Standing"),
	Crouching = 2 UMETA(DisplayName="Crouching"),
	Proning = 3 UMETA(DisplayName="Proning"),
	Sliding = 4 UMETA(DisplayName="Sliding")
};
