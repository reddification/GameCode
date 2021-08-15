// Fill out your copyright notice in the Description page of Project Settings.


#include "GCGameInstance.h"

#include "GCDebugSubsystem.h"

bool UGCGameInstance::ProcessConsoleExec(const TCHAR* Cmd, FOutputDevice& Ar, UObject* Executor)
{
	bool bResult = Super::ProcessConsoleExec(Cmd, Ar, Executor);
	if (!bResult)
	{
		TArray<UGameInstanceSubsystem*> Subsystems = GetSubsystemArray<UGameInstanceSubsystem>();
		for (const auto Subsystem : Subsystems)
		{
			if (Subsystem->ProcessConsoleExec(Cmd, Ar, Executor))
				break;
		}
	}
	
	return bResult;
}
