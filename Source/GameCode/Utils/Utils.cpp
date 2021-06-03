#include "Utils.h"
#include "DrawDebugHelpers.h"

namespace Utils
{
	void DrawDebugLineTrace(const UWorld* World, const FVector& Start, const FVector& End, EDrawDebugTrace::Type DrawDebugType,
		bool bHit, const FHitResult& OutHit, FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime, bool isThick)
	{
		if (DrawDebugType != EDrawDebugTrace::None)
		{
			bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
			float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

			// @fixme, draw line with thickness = 2.f?
			if (bHit && OutHit.bBlockingHit)
			{
				// Red up to the blocking hit, green thereafter
				::DrawDebugLine(World, Start, OutHit.ImpactPoint, TraceColor.ToFColor(true), bPersistent, LifeTime, 0, isThick ? 10 : 0);
				::DrawDebugLine(World, OutHit.ImpactPoint, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime, 0, isThick ? 10 : 0);
				::DrawDebugPoint(World, OutHit.ImpactPoint, 4, bHit ? TraceHitColor.ToFColor(true) : TraceColor.ToFColor(true), bPersistent, LifeTime);
			}
			else
			{
				// no hit means all red
				::DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
			}
		}
	}
}
