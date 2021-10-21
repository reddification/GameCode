#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "NavLinkProxyMantle.generated.h"

UCLASS()
class GAMECODE_API ANavLinkProxyMantle : public ANavLinkProxy
{
	GENERATED_BODY()
	
public:
	ANavLinkProxyMantle();
	
protected:
	virtual void BeginPlay() override;
	
private:
	UFUNCTION()
	void InvokeMantle(AActor* Actor, const FVector& DestinationPoint);
};
