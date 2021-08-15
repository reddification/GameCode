// Fill out your copyright notice in the Description page of Project Settings.


#include "SpiderPawn.h"

#include "Components/SphereComponent.h"
#include "GameCode/Components/Movement/SpiderPawnMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

ASpiderPawn::ASpiderPawn()
{
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);

	MovementComponent = CreateDefaultSubobject<UPawnMovementComponent, USpiderPawnMovementComponent>(
		TEXT("MovementComponent"));
	MovementComponent->SetUpdatedComponent(CollisionComponent);
	
	IKScale = GetActorScale().Z;
	IKTraceDistance = CollisionSphereRadius * IKScale;
}

void ASpiderPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	float rffo = GetIkOffsetForSocket(RightFrontFootSocketName);
	IKRightFrontFootOffset = FMath::FInterpTo(IKRightFrontFootOffset,rffo,DeltaSeconds, IKInterpSpeed);

	float lffo = GetIkOffsetForSocket(LeftFrontFootSocketName);
	IKLeftFrontFootOffset = FMath::FInterpTo(IKLeftFrontFootOffset, lffo, DeltaSeconds, IKInterpSpeed);

	float rrfo = GetIkOffsetForSocket(RightRearFootSocketName);
	IKRightRearFootOffset = FMath::FInterpTo(IKRightRearFootOffset,rrfo,DeltaSeconds, IKInterpSpeed);

	float lrfo = GetIkOffsetForSocket(LeftRearFootSocketName);
	IKLeftRearFootOffset = FMath::FInterpTo(IKLeftRearFootOffset, lrfo,DeltaSeconds, IKInterpSpeed);

	bool bFalling = rffo > MaxLegExtend && lffo > MaxLegExtend && rrfo > MaxLegExtend && lrfo > MaxLegExtend;
	static_cast<USpiderPawnMovementComponent*>(MovementComponent)->SetIsFalling(bFalling);
}

float ASpiderPawn::GetIkOffsetForSocket(const FName& SocketName)
{
	FVector SocketLocation = SkeletalMeshComponent->GetSocketLocation(SocketName);
	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	//approximate. IKTraceDistance is actually collision sphere radius which is expected to end at foot level (see related BP)
	FVector FootPosition = TraceStart - IKTraceDistance * FVector::UpVector;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	FHitResult HitResult;
	
	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), TraceStart, FootPosition - IKTraceExtendDistance * FVector::UpVector,
		TraceType, true,TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, HitResult, true);

	return bHit
		? (FootPosition.Z - HitResult.Location.Z) / IKScale
		: IKHangingFootOffset;
}
