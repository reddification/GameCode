#include "NavLinkProxySlide.h"

#include "NavLinkCustomComponent.h"
#include "Characters/GCBaseCharacter.h"

ANavLinkProxySlide::ANavLinkProxySlide()
{
	bSmartLinkIsRelevant = true;
}

void ANavLinkProxySlide::BeginPlay()
{
	Super::BeginPlay();
	OnSmartLinkReached.AddDynamic(this, &ANavLinkProxySlide::InvokeSlide);
}

void ANavLinkProxySlide::InvokeSlide(AActor* Actor, const FVector& DestinationPoint)
{
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Actor);
	if (IsValid(Character))
	{
		Character->TryStartSliding();
	}
}

void ANavLinkProxySlide::OnSlideEnded()
{
}