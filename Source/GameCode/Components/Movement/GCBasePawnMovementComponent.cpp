// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBasePawnMovementComponent.h"

void UGCBasePawnMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	if (ShouldSkipUpdate(DeltaTime))
		return;
	
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Move(DeltaTime);
}

void UGCBasePawnMovementComponent::JumpStart()
{
	VerticalSpeed = InitialJumpSpeed;
}

bool UGCBasePawnMovementComponent::IsFalling() const
{
	return bIsFalling;
}

void UGCBasePawnMovementComponent::Move(float DeltaTime)
{
	FVector PendingInput = GetPendingInputVector().GetClampedToMaxSize(1.0f);
	Velocity = PendingInput * MaxSpeed;
	ConsumeInputVector();

	if (bEnableGravity)
	{
		FHitResult hit;
		const float TraceDepth = 1.05f;
		FVector traceStartLocation = UpdatedComponent->GetComponentLocation();
		FVector traceEndLocation = traceStartLocation - TraceDepth * FVector::UpVector;
		FCollisionQueryParams traceParams;
		traceParams.AddIgnoredActor(GetOwner());
		bool bWasFalling = bIsFalling;
		FCollisionShape Sphere = FCollisionShape::MakeSphere(PawnHalfHeight * TraceDepth);
		bIsFalling = !GetWorld()->SweepSingleByChannel(hit, traceStartLocation, traceEndLocation,
			FQuat::Identity, ECC_Visibility, Sphere, traceParams);

		if (bIsFalling)
		{
			VerticalSpeed += GetGravityZ() * DeltaTime;
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, TEXT("Falling"));
		}
		else if (bWasFalling && VerticalSpeed < 0)
		{
			VerticalSpeed = 0;
			GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Not falling"));
		}

		Velocity.Z += VerticalSpeed;
	}
	
	FVector Delta = Velocity * DeltaTime; // h = h0 + v0t 
	if (!Delta.IsNearlyZero(1e-6f))
	{
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			HandleImpact(Hit, DeltaTime, Delta);
			// Try to slide the remaining distance along the surface.
			SlideAlongSurface(Delta, 1.f-Hit.Time, Hit.Normal, Hit, true);
		}
	}

	UpdateComponentVelocity();
}
