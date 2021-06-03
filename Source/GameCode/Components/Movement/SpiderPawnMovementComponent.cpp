// Fill out your copyright notice in the Description page of Project Settings.


#include "SpiderPawnMovementComponent.h"

void USpiderPawnMovementComponent::JumpStart()
{
	VerticalSpeed = InitialJumpSpeed;
}

bool USpiderPawnMovementComponent::IsFalling() const
{
	return bIsFalling;
}

void USpiderPawnMovementComponent::Move(float DeltaTime)
{
	FVector PendingInput = GetPendingInputVector().GetClampedToMaxSize(1.0f);
	Velocity = PendingInput * MaxSpeed;
	ConsumeInputVector();

	if (bIsFalling)
	{
		VerticalSpeed += GetGravityZ() * DeltaTime;
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Green, TEXT("Falling"));
	}
	else if (VerticalSpeed < 0)
	{
		VerticalSpeed = 0;
		GEngine->AddOnScreenDebugMessage(-1, 2, FColor::Yellow, TEXT("Not falling"));
	}

	Velocity.Z += VerticalSpeed;
	
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
