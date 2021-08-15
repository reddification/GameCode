// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCameras, Verbose, All)
#define ECC_Climbable ECC_GameTraceChannel1
#define ECC_Interactable ECC_GameTraceChannel2
#define ECC_Wallrunnable ECC_GameTraceChannel3
const FName DebugCategoryLedgeDetection = FName("LedgeDetection");
const FName DebugCategoryWallrun = FName("Wallrun");
const FName ProfilePawn = FName("Pawn");
const FName ProfileInteractionVolume = FName("PawnInteractionVolume");