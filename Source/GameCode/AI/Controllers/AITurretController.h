
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "GCAIController.h"
#include "AITurretController.generated.h"

UCLASS()
class GAMECODE_API AAITurretController : public AGCAIController
{
	GENERATED_BODY()

public:
	AAITurretController();
	
	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

protected:
	// in seconds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ThinkDelay = 0.2;

private:
	TWeakObjectPtr<class ATurret> ControlledTurret;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void Think();
	void OnDeath();

	TTuple<AActor*, FAIStimulus> MostDangerousActorInfo;
	FTimerHandle ThinkTimer;
	TArray<AActor*> CurrentlySightPerceptedActors;
};
