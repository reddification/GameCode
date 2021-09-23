#pragma once

struct FIkData
{
	float RightFootElevation = 0;
	float RightFootPitch = 0;
	float RightKneeOutwardExtend = 0;
	
	float LeftFootElevation = 0;
	float LeftFootPitch = 0;
	float LeftKneeOutwardExtend = 0;
	
	float PelvisElevation = 0;
	
	float IKScale = 1;

	void LogFeetElevations(bool bAdjusted) const;
	void LogFeetPivots() const;
	void LogKneeExtends() const;
};
