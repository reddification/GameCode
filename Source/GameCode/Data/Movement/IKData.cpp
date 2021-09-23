#include "IKData.h"

void FIkData::LogFeetElevations(bool bAdjusted) const
{
	const FColor LogColor = bAdjusted ? FColor::Yellow : FColor::Blue;
	const int KeyOffset = bAdjusted ? 2 : 0;
	const FString AdjustmentPrefix = bAdjusted ? "Adjusted " : "";
	GEngine->AddOnScreenDebugMessage(1 + KeyOffset, 2, LogColor,
		FString::Printf(TEXT("%s Right foot IK offset :%f"), *AdjustmentPrefix, RightFootElevation));
	GEngine->AddOnScreenDebugMessage(2 + KeyOffset, 2, LogColor,
		FString::Printf(TEXT("%s Left foot IK offset :%f"), *AdjustmentPrefix, LeftFootElevation));

	if (bAdjusted)
	{
		GEngine->AddOnScreenDebugMessage(5, 2, LogColor,
		FString::Printf(TEXT("Pelvis IK offset :%f"), PelvisElevation));
	}
}

void FIkData::LogFeetPivots() const
{
	GEngine->AddOnScreenDebugMessage(8, 2, FColor::Purple,
			FString::Printf(TEXT("Left foot pitch: %f"), LeftFootPitch));

	GEngine->AddOnScreenDebugMessage(9, 2, FColor::Purple,
	FString::Printf(TEXT("Right foot pitch: %f"),RightFootPitch));	
}

void FIkData::LogKneeExtends() const
{
	GEngine->AddOnScreenDebugMessage(10, 2, FColor::Orange,
		FString::Printf(TEXT("Left knee outward extend: %f"), LeftKneeOutwardExtend));

	GEngine->AddOnScreenDebugMessage(11, 2, FColor::Orange,
		FString::Printf(TEXT("Right knee outward extend: %f"), RightKneeOutwardExtend));
}