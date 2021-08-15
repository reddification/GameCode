#include "GCTraceUtils.h"

#include "DrawDebugHelpers.h"

bool GCTraceUtils::SweepCapsuleSingleByChannel(const UWorld* const World, FHitResult& OutHit, const FVector& Start,
                                               const FVector& End, float CapsuleRadius, float CapsuleHalfHeight, ECollisionChannel TraceChannel,
                                               const FCollisionQueryParams& Params, const FTraceParams& TraceParams, const FQuat& Rot,
                                               const FCollisionResponseParams& ResponseParam)
{
	const FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	const bool bHit = World->SweepSingleByChannel(OutHit, Start, End, Rot, TraceChannel, CapsuleShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if (TraceParams.bDrawDebug)
	{
		DrawDebugCapsule(World, Start, CapsuleHalfHeight, CapsuleRadius,FQuat::Identity, TraceParams.TraceColor,
			false, TraceParams.DrawTime);
		DrawDebugCapsule(World, End, CapsuleHalfHeight, CapsuleRadius,FQuat::Identity, TraceParams.TraceColor,
			false, TraceParams.DrawTime);
		DrawDebugLine(World, Start, End, FColor::Yellow, false,	TraceParams.DrawTime, 0, 2);
		if (bHit)
		{
			DrawDebugCapsule(World, OutHit.Location, CapsuleHalfHeight, CapsuleRadius,
		FQuat::Identity, TraceParams.HitColor, false, TraceParams.DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, TraceParams.HitColor, false,
				TraceParams.DrawTime);	
		}
	}
#endif

	return bHit;
}

bool GCTraceUtils::SweepSphereSingleByChannel(const UWorld* const World, FHitResult& OutHit, const FVector& Start,
	const FVector& End, float Radius, ECollisionChannel TraceChannel,
	const FCollisionQueryParams& Params, const FTraceParams& TraceParams, const FQuat& Rot,
	const FCollisionResponseParams& ResponseParam)
{
	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(Radius);
	const bool bHit = World->SweepSingleByChannel(OutHit, Start, End, Rot, TraceChannel, SphereShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if (TraceParams.bDrawDebug)
	{
		const FVector TraceVector = End-Start;
		float HalfHeight = TraceVector.Size() * 0.5f + Radius;
		const FQuat Quat = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();
		// const FQuat Quat = TraceVector.ToOrientationQuat(); // same???
	
		DrawDebugSphere(World, Start, Radius, 16, TraceParams.TraceColor,false, TraceParams.DrawTime);
		DrawDebugSphere(World, End, Radius, 16, TraceParams.TraceColor,false, TraceParams.DrawTime);
		DrawDebugLine(World, Start, End, FColor::Yellow, false,	TraceParams.DrawTime, 0, 2);

		DrawDebugCapsule(World, (Start + End) * 0.5, HalfHeight, Radius, Quat,
			FColor::Yellow,false, TraceParams.DrawTime);
		
		if (bHit)
		{
			DrawDebugSphere(World, OutHit.Location, Radius, 16, TraceParams.HitColor,false, TraceParams.DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.f, FColor::Green, false, TraceParams.DrawTime);
		}
	}
#endif
	
	return bHit;
}

bool GCTraceUtils::OverlapCapsuleAnyByProfile(const UWorld* World, const FVector& Location, float Radius, float HalfHeight,
	FName Profile, const FCollisionQueryParams& QueryParams, const FTraceParams& TraceParams, const FQuat& Quat)
{
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	bool bOverlap = World->OverlapAnyTestByProfile(Location, Quat, Profile, CollisionShape, QueryParams);

#if ENABLE_DRAW_DEBUG
	if (bOverlap && TraceParams.bDrawDebug)
	{
		DrawDebugCapsule(World, Location, HalfHeight, Radius, Quat, FColor::Cyan, false,
			TraceParams.DrawTime, 0, 3);
	}
#endif
	
	return bOverlap;
}

bool GCTraceUtils::OverlapCapsuleBlockingByProfile(const UWorld* World, const FVector& Location, float Radius, float HalfHeight,
	FName Profile, const FCollisionQueryParams& QueryParams, const FTraceParams& TraceParams, const FQuat& Quat)
{
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(Radius, HalfHeight);
	bool bOverlap = World->OverlapBlockingTestByProfile(Location, Quat, Profile, CollisionShape, QueryParams);

#if ENABLE_DRAW_DEBUG
	if (bOverlap && TraceParams.bDrawDebug)
	{
		DrawDebugCapsule(World, Location, HalfHeight, Radius, Quat, FColor::Cyan, false,
			TraceParams.DrawTime, 0, 3);
	}
#endif
	
	return bOverlap;
}