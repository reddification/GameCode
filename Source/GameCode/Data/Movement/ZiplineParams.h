#pragma once

struct FZiplineParams
{
	FVector ZiplineNormalizedDirection = FVector::ZeroVector;
	FVector InitialVelocity = FVector::ZeroVector;
	FPlane MovementPlane;
	float DeclinationAngle = 0.f;
	float DeclinationAngleSin = 0.f;
	float DeclinationAngleCos = 0.f;
	float Friction = 0.f;
	float CurrentSpeed = 0.f;
	FVector CorrectedActorLocation = FVector::ZeroVector;
};
