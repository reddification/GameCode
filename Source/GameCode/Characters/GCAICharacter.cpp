#include "Characters/GCAICharacter.h"

#include "NavigationInvokerComponent.h"
#include "Components/AI/AIPatrolComponent.h"

AGCAICharacter::AGCAICharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIPatrolComponent = CreateDefaultSubobject<UAIPatrolComponent>(TEXT("PatrolComponent"));
	AddOwnedComponent(AIPatrolComponent);

	// NavigationInvokerComponent = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavigationInvoker"));
	// AddOwnedComponent(NavigationInvokerComponent);
}
