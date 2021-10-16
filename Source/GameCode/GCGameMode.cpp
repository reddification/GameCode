#include "GCGameMode.h"

#include "GenericTeamAgentInterface.h"
#include "Data/AITypesGC.h"

void AGCGameMode::StartPlay()
{
	Super::StartPlay();
	FGenericTeamId::SetAttitudeSolver(GCTeamAttitudeSolver);
}
