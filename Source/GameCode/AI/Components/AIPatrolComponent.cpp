#include "AIPatrolComponent.h"
#include "Actors/Navigation/PatrolPath.h"

FVector UAIPatrolComponent::SelectClosestWaypoint()
{
	FVector OwnerLocation = GetOwner()->GetActorLocation();
	const TArray<FVector>& Waypoints = PatrolPath->GetWaypoints();
	const FTransform& PatrolPathTransform = PatrolPath->GetActorTransform();
	FVector ClosestWaypoint;
	float MinDistanceSq = FLT_MAX;
	for (auto i = 0; i < Waypoints.Num(); i++)
	{
		const FVector& Waypoint = Waypoints[i];
		const FVector& WorldWaypointLocation = PatrolPathTransform.TransformPosition(Waypoint);
		float CurrentSqDistance = (OwnerLocation - WorldWaypointLocation).SizeSquared();
		if (CurrentSqDistance < MinDistanceSq)
		{
			MinDistanceSq = CurrentSqDistance;
			ClosestWaypoint = WorldWaypointLocation;
			SelectedWaypointIndex = i;
		}
	}

	return ClosestWaypoint;
}

FVector UAIPatrolComponent::SelectNextWaypoint()
{
	const TArray<FVector>& Waypoints = PatrolPath->GetWaypoints();
	SelectedWaypointIndex = (SelectedWaypointIndex + 1) % Waypoints.Num();
	const FTransform& PatrolPathTransform = PatrolPath->GetActorTransform();
	return PatrolPathTransform.TransformPosition(Waypoints[SelectedWaypointIndex]);
}

bool UAIPatrolComponent::CanPatrol() const
{
	return IsValid(PatrolPath) && PatrolPath->GetWaypoints().Num() > 0;
}
