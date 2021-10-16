#pragma once

#include "GenericTeamAgentInterface.h"
#include "AITypesGC.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Player,
	GoodGuys,
	BadGuys,
	Dead
};

inline ETeamAttitude::Type GCTeamAttitudeSolver(FGenericTeamId A, FGenericTeamId B)
{
	if (A.GetId() ==(uint8)ETeam::Dead || B.GetId() == (uint8)ETeam::Dead)
		return ETeamAttitude::Neutral;
	
	return A != B ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
}