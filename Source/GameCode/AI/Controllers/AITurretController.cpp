#include "AITurretController.h"
#include "AI/Characters/Turret.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/Character/CharacterAttributesComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Sight.h"

AAITurretController::AAITurretController()
{
	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AAITurretController::OnTargetPerceptionUpdated);
}

void AAITurretController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	if (IsValid(InPawn))
	{
		checkf(InPawn->IsA<ATurret>(), TEXT("AAITurretController is intended to be used only with ATurret pawn"))
		ControlledTurret = StaticCast<ATurret*>(InPawn);
		GetWorld()->GetTimerManager().SetTimer(ThinkTimer, this, &AAITurretController::Think, ThinkDelay, true, ThinkDelay);
		ControlledTurret->DeathEvent.AddUObject(this, &AAITurretController::OnDeath);
	}
}

void AAITurretController::ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	Super::ActorsPerceptionUpdated(UpdatedActors);
	if (!ControlledTurret.IsValid())
	{
		return;
	}
	
	CurrentlySightPerceptedActors.Empty(); 
	PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlySightPerceptedActors);

	Think();
}

void AAITurretController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	TSubclassOf<UAISense> StimulusClass = UAIPerceptionSystem::GetSenseClassForStimulus(GetWorld(), Stimulus);
	if (StimulusClass == UAISense_Damage::StaticClass())
	{
		if (Stimulus.IsExpired())
		{
			if (Actor == MostDangerousActorInfo.Key)
			{
				MostDangerousActorInfo.Key = nullptr;
			}
		}
		else if (!IsValid(MostDangerousActorInfo.Key) || MostDangerousActorInfo.Value.Strength < Stimulus.Strength)
		{
			ControlledTurret->SetCurrentTarget(Actor);
			MostDangerousActorInfo.Key = Actor;
			MostDangerousActorInfo.Value = Stimulus;
		}
	}
}

void AAITurretController::Think()
{
	if (IsValid(MostDangerousActorInfo.Key))
	{
		return;
	}
	
	AActor* ClosestActor = nullptr;
	float MinSquaredDistance = FLT_MAX;
	FVector TurretLocation = ControlledTurret->GetActorLocation();
	// I guess without a copy if actors are updated by perception component, foreach loop can actually crash here
	TArray<AActor*> SightPerceptedActorsCopy = CurrentlySightPerceptedActors;
	for (AActor* PerceivedActor : SightPerceptedActorsCopy)
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

	// AActor* ClosestActor = GetClosestSensedActor(UAISense_Sight::StaticClass());
	ControlledTurret->SetCurrentTarget(ClosestActor);
}

void AAITurretController::OnDeath()
{
	PerceptionComponent->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
	PerceptionComponent->SetSenseEnabled(UAISense_Damage::StaticClass(), false);
}
