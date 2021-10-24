#pragma once
#include "Navigation/NavLinkProxy.h"
#include "NavLinkCustomComponent.h"
#include "NavLinkProxySlide.generated.h"

UCLASS()
class GAMECODE_API ANavLinkProxySlide : public ANavLinkProxy
{
	GENERATED_BODY()
	
public:
	ANavLinkProxySlide();

protected:
	virtual void BeginPlay() override;

private:
	UNavLinkCustomComponent::FOnMoveReachedLink OnMoveReachedLink;
	void OnSlideEnded();

	UFUNCTION()
	void InvokeSlide(AActor* Actor, const FVector& DestinationPoint);
};
