#include "MeleeHitRegistratorComponent.h"

#include "GameCode.h"
#include "GCDebugSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Utils/DebugUtils.h"
#include "Utils/GCTraceUtils.h"

UMeleeHitRegistratorComponent::UMeleeHitRegistratorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SphereRadius = 5.f;
	SetCollisionProfileName(ProfileNoCollision);
}

void UMeleeHitRegistratorComponent::TickComponent(float DeltaTime, ELevelTick Tick,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, Tick, ThisTickFunction);
	if (bEnabled)
	{
		ProcessHitRegistration();
	}
	// order is important
	PreviousLocation = GetComponentLocation();
}

void UMeleeHitRegistratorComponent::ProcessHitRegistration()
{
	#if ENABLE_DRAW_DEBUG
		bool bDrawDebugEnabled = GetDebugSubsystem(GetWorld())->IsDebugCategoryEnabled(DebugCategoryMeleeWeapons);
	#else
		bool bDrawDebugEnabled = false;
	#endif

	
	FVector CurrentLocation = GetComponentLocation();
	FCollisionQueryParams QueryParams;
	AActor* Owner = GetOwner();
	while (Owner != nullptr)
	{
		QueryParams.AddIgnoredActor(Owner);
		Owner = Owner->GetOwner();
	}

	GCTraceUtils::FTraceParams TraceParams(bDrawDebugEnabled);
	FHitResult Hit;
	// TODO SweepSphereMultiByChannel?
	bool bHit = GCTraceUtils::SweepSphereSingleByChannel(GetWorld(), Hit, PreviousLocation, CurrentLocation, GetScaledSphereRadius(),
		ECC_MeleeHitRegistrator, QueryParams, TraceParams);

	if (bHit)
	{
		FVector Direction = (CurrentLocation - PreviousLocation).GetSafeNormal();
		MeleeHitRegisteredEvent.ExecuteIfBound(Hit, Direction);
	}
}