#pragma once

#include "CoreMinimal.h"
#include "Characters/GCBaseCharacter.h"
#include "GCAICharacter.generated.h"

class UAIPatrolComponent;

UCLASS(Blueprintable)
class GAMECODE_API AGCAICharacter : public AGCBaseCharacter
{
	GENERATED_BODY()

public:
	AGCAICharacter(const FObjectInitializer& ObjectInitializer);

	UAIPatrolComponent* GetAIPatrolComponent() const { return AIPatrolComponent; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAIPatrolComponent* AIPatrolComponent;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	// class UNavigationInvokerComponent* NavigationInvokerComponent;
};
