#pragma once

UENUM()
enum class EStopClimbingMethod : uint8
{
	Fall = 0,
	ReachingTop,
	ReachingBottom,
	JumpOff
};
