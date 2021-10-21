#include "Characters/Controllers/AITurretController.h"

#include "Characters/GCBaseCharacter.h"
#include "Characters/Turret.h"
#include "Perception/AISense_Sight.h"

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController is intended to be used only with ATurret pawn"))
		ControlledTurret = StaticCast<ATurret*>(InPawn);
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!ControlledTurret.IsValid())
	{
		return;
	}
	
	AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	ControlledTurret->SetCurrentTarget(ClosestActor);
}