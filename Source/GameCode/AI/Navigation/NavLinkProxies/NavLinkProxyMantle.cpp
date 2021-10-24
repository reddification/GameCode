#include "AI/Navigation/NavLinkProxies/NavLinkProxyMantle.h"

#include "NavLinkCustomComponent.h"
#include "Characters/GCBaseCharacter.h"


ANavLinkProxyMantle::ANavLinkProxyMantle()
{
	bSmartLinkIsRelevant = true;
}

void ANavLinkProxyMantle::BeginPlay()
{
	Super::BeginPlay();
	OnSmartLinkReached.AddDynamic(this, &ANavLinkProxyMantle::InvokeMantle);
}

void ANavLinkProxyMantle::InvokeMantle(AActor* Actor, const FVector& DestinationPoint)
{
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Actor);
	if (IsValid(Character))
	{
		Character->Mantle();
	}
}
