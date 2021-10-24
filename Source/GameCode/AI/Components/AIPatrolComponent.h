#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIPatrolComponent.generated.h"

class APatrolPath;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UAIPatrolComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	FVector SelectClosestWaypoint();
	FVector SelectNextWaypoint();
	bool CanPatrol() const;
	
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	APatrolPath* PatrolPath;

private:
	int32 SelectedWaypointIndex = -1;
};
