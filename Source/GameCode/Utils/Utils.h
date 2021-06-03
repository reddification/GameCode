#pragma once
#include <Kismet/KismetSystemLibrary.h>
namespace Utils
{
	void DrawDebugLineTrace(const UWorld* World, const FVector& Start, const FVector& End, EDrawDebugTrace::Type DrawDebugType,
        bool bHit, const FHitResult& OutHit, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime, bool isThick);
}