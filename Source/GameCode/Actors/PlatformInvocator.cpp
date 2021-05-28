// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformInvocator.h"

APlatformInvocator::APlatformInvocator()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent = DefaultPlatformRoot;
}

void APlatformInvocator::Invoke()
{
	if (OnInvocatorActivated.IsBound())
	{
		OnInvocatorActivated.Broadcast();
	}
}
