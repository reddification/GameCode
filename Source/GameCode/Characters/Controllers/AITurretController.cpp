#include "Characters/Controllers/AITurretController.h"

#include "Characters/GCBaseCharacter.h"
#include "Characters/Turret.h"
#include "Components/Character/CharacterAttributesComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

AAITurretController::AAITurretController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("TurretPerception"));
}

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController is intended to be used only with ATurret pawn"))
		ControlledTurret = StaticCast<ATurret*>(InPawn);
	}
	else
	{
		ControlledTurret = nullptr;
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!ControlledTurret.IsValid())
	{
		return;
	}

	
	TArray<AActor*> CurrentlyPerceivedActors;
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlyPerceivedActors);
	AActor* ClosestActor = nullptr;
	float MinSquaredDistance = FLT_MAX;
	FVector TurretLocation = ControlledTurret->GetActorLocation();
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
		float CurrentSquaredDistance = FVector::DistSquared(PerceivedActor->GetActorLocation(), TurretLocation);
		if (CurrentSquaredDistance < MinSquaredDistance)
		{
			MinSquaredDistance = CurrentSquaredDistance;
			ClosestActor = PerceivedActor;
		}
	}

	ControlledTurret->SetCurrentTarget(ClosestActor);
}