// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ExplosionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExplosionEvent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UExplosionComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Explode(AController* Controller);
	
	UPROPERTY(BlueprintAssignable)
	FExplosionEvent ExplosionEvent;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	float MaxDamage = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 1, ClampMin = 1))
	float MinDamage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	float DamageFalloff = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	float InnerRadius = 1000.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(UIMin = 0, ClampMin = 0))
	float OuterRadius = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UParticleSystem* ExplosionVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class USoundCue* ExplosionSFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UDamageType> DamageTypeClass;
};
