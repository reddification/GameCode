#pragma once

#include "GCDebugSubsystem.h"
#include "Kismet/GameplayStatics.h"

#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG

static TWeakObjectPtr<UGCDebugSubsystem> DebugSubsystem = nullptr;

const UGCDebugSubsystem* GetDebugSubsystem(UWorld* World)
{
	if (!DebugSubsystem.Get())
	{
		DebugSubsystem = UGameplayStatics::GetGameInstance(World)->GetSubsystem<UGCDebugSubsystem>();
	}

	return DebugSubsystem.Get();
}

#endif	
