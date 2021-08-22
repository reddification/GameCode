#pragma once

struct FSlideData
{
	float Speed = 0.f;
	float VerticalSpeed = 0.f;
	float FloorAngle = 0.f;
	float FloorAngleCos = 0.f;
	float FloorAngleSin = 0.f;
	FTimerHandle TimerHandle;
	FTimerHandle CooldownTimerHandle;
	bool bCanSlide = true;
};
