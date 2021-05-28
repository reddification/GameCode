// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformInvocator.generated.h"

//why make it dynamic if it is not going to be bound in BPs?
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvocatorActivated);

UCLASS()
class GAMECODE_API APlatformInvocator : public AActor
{
	GENERATED_BODY()
	
public:
	APlatformInvocator();
	
	UPROPERTY(BlueprintAssignable, Category=PlatformInvocation)
	FOnInvocatorActivated OnInvocatorActivated;
	
protected:
	
	UFUNCTION(BlueprintCallable, Category=PlatformInvocation)
	void Invoke();	
};
