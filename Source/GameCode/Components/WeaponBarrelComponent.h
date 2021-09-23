// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCDebugSubsystem.h"
#include "Components/SceneComponent.h"
#include "Data/DecalSettings.h"
#include "WeaponBarrelComponent.generated.h"

class UNiagaraSystem;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UWeaponBarrelComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	void Shoot(const FVector& ViewLocation, const FVector& Direction, AController* ShooterController);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Range = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float InitialDamage = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* MuzzleFlashFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UNiagaraSystem* TraceFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FDecalSettings DecalSettings; 

	// MUst be normalized 0..1 on both axis. Curve value will be multiplied by InitialDamage and applied to actor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveFloat* DamageFalloffDiagram;
	
private:

	#if UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
	const UGCDebugSubsystem* GetDebugSubsystem() const;
	mutable UGCDebugSubsystem* DebugSubsystem;
	#endif
};
