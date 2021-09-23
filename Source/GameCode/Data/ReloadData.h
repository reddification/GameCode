#pragma once

class UAnimMontage;
struct FReloadData
{
	FReloadData()
		: bStarted(false)
	{
	}
	
	FReloadData(UAnimMontage* reloadMontage, float desiredDuration)
		: CharacterReloadMontage(reloadMontage), DesiredDuration(desiredDuration), bStarted(true)
	{
	}

	bool bStarted;
	UAnimMontage* CharacterReloadMontage;
	float DesiredDuration;
};