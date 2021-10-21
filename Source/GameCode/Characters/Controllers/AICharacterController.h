#pragma once

#include "CoreMinimal.h"
#include "Characters/Controllers/GCAIController.h"
#include "AICharacterController.generated.h"

UCLASS()
class GAMECODE_API AAICharacterController : public AGCAIController
{
	GENERATED_BODY()

public:
	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ActorTargetReachRadius = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float WaypointTargetReachRadius = 20.f;

	// in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ActorFollowCooldown = 0.1f;
	
private:
	void TryPatrol();
	void TryMoveToNextTarget();
	bool IsTargetReached(FVector TargetLocation, float TargetReachRadius) const;
	void OnFollowCooldownElapsed();
	
	TWeakObjectPtr<class AGCAICharacter> ControlledCharacter;
	TWeakObjectPtr<AActor> LastFollowedActor;
	FTimerHandle ActorFollowCooldownTimer;

	bool bCanFollowActor = true;
	bool bPatrolling = false;
};
