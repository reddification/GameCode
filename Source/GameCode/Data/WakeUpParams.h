#pragma once

struct FWakeUpParams
{
	float ComponentScale = 0.f;
	float SweepInflation = 0.f;
	FVector PawnLocation = FVector::ZeroVector;
	FCollisionQueryParams CollisionQueryParams;
	FCollisionResponseParams ResponseParam;
	ECollisionChannel CollisionChannel;
	class UCapsuleComponent* CapsuleComponent;
	class ACharacter* DefaultCharacter;
};