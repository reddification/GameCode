#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MeleeHitRegistratorComponent.generated.h"

DECLARE_DELEGATE_TwoParams(FMeleeHitRegisteredEvent, const FHitResult& Hit, const FVector& HitDirection);

UCLASS(meta=(BlueprintSpawnableComponent))
class GAMECODE_API UMeleeHitRegistratorComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UMeleeHitRegistratorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ProcessHitRegistration();

	void SetIsEnabled(bool bNewValue) { bEnabled = bNewValue; }

	mutable FMeleeHitRegisteredEvent MeleeHitRegisteredEvent;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = false;

private:
	FVector PreviousLocation = FVector::ZeroVector;
};
