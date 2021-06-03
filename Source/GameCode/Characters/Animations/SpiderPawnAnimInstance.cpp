// Fill out your copyright notice in the Description page of Project Settings.


#include "SpiderPawnAnimInstance.h"
#include "GameCode/Characters/SpiderPawn.h"

void USpiderPawnAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	auto pawn = TryGetPawnOwner();
	checkf(pawn->IsA<ASpiderPawn>(), TEXT("Only spider bot pawn supported for SpiderPawnAnimInstance"));
	SpiderPawnOwner = static_cast<ASpiderPawn*>(pawn);
}

void USpiderPawnAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!SpiderPawnOwner.IsValid())
		return;

	RightFrontFootEffectorLocation = FVector(SpiderPawnOwner->GetIKRightFrontFootOffset(), 0, 0);
	LeftFrontFootEffectorLocation = FVector(SpiderPawnOwner->GetIKLeftFrontFootOffset(), 0, 0);
	RightRearFootEffectorLocation = FVector(SpiderPawnOwner->GetIKRightRearFootOffset(), 0, 0);
	LeftRearFootEffectorLocation = FVector(SpiderPawnOwner->GetIKLeftRearFootOffset(), 0, 0);
}
