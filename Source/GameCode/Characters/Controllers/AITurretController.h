#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AITurretController.generated.h"

UCLASS()
class GAMECODE_API AAITurretController : public AAIController
{
	GENERATED_BODY()

public:
	AAITurretController();

	virtual void SetPawn(APawn* InPawn) override;

	virtual void ActorsPerceptionUpdated(const TArray<AActor*>& UpdatedActors) override;

private:
	TWeakObjectPtr<class ATurret> ControlledTurret;
};
