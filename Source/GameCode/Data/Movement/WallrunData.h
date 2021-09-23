#pragma once
#include "GCMovementMode.h"

class UCurveFloat;

struct FWallrunData
{
	UCurveFloat* ActiveHeightDynamicCurve = nullptr;
	float InitialWorldZ = 0.f;
	FVector SurfaceNormal = FVector::ZeroVector;
	ESide Side = ESide::None;
    float Progress = 0.f;
    bool bWantsToWallrun = false;
	float HeightCurveValue = 0.f;
	float SurfaceRollAngle = 0.f;

	float GetSideModificator(const ESide& WallrunSide) const
	{
		switch (WallrunSide)
		{
			case ESide::Left:
				return -1.f;
			case ESide::Right:
				return 1.f;
			default:
				return 0.f;
		}
	}
};