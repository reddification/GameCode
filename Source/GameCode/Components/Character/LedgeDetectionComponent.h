// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LedgeDetectionComponent.generated.h"

USTRUCT(BlueprintType)
struct FLedgeDescriptor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector Location;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FRotator Rotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector LedgeNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MantlingHeight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* MantleTarget;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API ULedgeDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	bool DetectLedge(OUT FLedgeDescriptor& LedgeDescriptor);
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", meta=(UIMin=0.0f, ClampMin=0.0f))
	float MinLedgeHeight = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", meta=(UIMin=0.0f, ClampMin=0.0f))
	float MaxLedgeHeight = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings", meta=(UIMin=0.0f, ClampMin=0.0f))
	float ForwardCheckDistance = 50.f;	

private:
	class ACharacter* CharacterOwner;
	class UGCDebugSubsystem* DebugSubsystem;

	UGCDebugSubsystem* GetDebugSubsystem();
};
