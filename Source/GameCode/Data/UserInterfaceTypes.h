#pragma once

UENUM(BlueprintType)
enum class EReticleType : uint8
{
	None UMETA(DisplayName="None"),
	Crosshair UMETA(DisplayName="Crosshair"),
	Scope UMETA(DisplayName="Scope"),
	MAX UMETA(Hidden)
};