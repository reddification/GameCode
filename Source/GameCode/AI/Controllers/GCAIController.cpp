#include "GCAIController.h"

#include "Characters/GCBaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"

AGCAIController::AGCAIController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
}

AActor* AGCAIController::GetClosestSensedActor(TSubclassOf<UAISense> SenseType) const
{
	if (!IsValid(GetPawn()))
	{
		return nullptr;
	}
	
	TArray<AActor*> CurrentlyPerceivedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(SenseType, CurrentlyPerceivedActors);
	if (CurrentlyPerceivedActors.Num() <= 0)
	{
		return nullptr;
	}
	
	AActor* ClosestActor = nullptr;
	float MinSquaredDistance = FLT_MAX;
	FVector PawnLocation = GetPawn()->GetActorLocation();
	for (AActor* PerceivedActor : CurrentlyPerceivedActors)
	{
		AGCBaseCharacter* PerceivedCharacter = Cast<AGCBaseCharacter>(PerceivedActor);
		if (IsValid(PerceivedCharacter))
		{
			if (!PerceivedCharacter->GetCharacterAttributesComponent()->IsAlive())
			{
				continue;
			}
		}
		
		float CurrentSquaredDistance = FVector::DistSquared(PerceivedActor->GetActorLocation(), PawnLocation);
		if (CurrentSquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = CurrentSquaredDistance;
			ClosestActor = PerceivedActor;
		}
	}

	return ClosestActor;
}
