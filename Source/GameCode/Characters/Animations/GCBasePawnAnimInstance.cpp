// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBasePawnAnimInstance.h"

#include "GameCode/Characters/GCBasePawn.h"
#include "GameFramework/PawnMovementComponent.h"

void UGCBasePawnAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	checkf(TryGetPawnOwner()->IsA<AGCBasePawn>(), TEXT("GCBasePawnAnimInstance Only GC Base Pawn supported"))
	Pawn = static_cast<AGCBasePawn*>(TryGetPawnOwner());
}

void UGCBasePawnAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	// UpdateAnimation can be called even before BeginPlay
	if (!Pawn.IsValid())
		return;
	
	InputForward = Pawn->GetInputForward();
	InputRight = Pawn->GetInputRight();
	bInAir = Pawn->MovementComponent->IsFalling();
}
