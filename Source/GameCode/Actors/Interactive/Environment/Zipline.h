// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCode/Actors/Interactive/InteractiveActor.h"

#include "Zipline.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAMECODE_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()

public:
	AZipline();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual EInteractionType GetInteractionType() const override { return EInteractionType::Ziplining; }
	FVector GetZiplineDirection() const;
	float GetDeclinationAngle() const;
	float GetCableFriction() const { return CableFriction; }
	FVector GetTopPoleWorldLocation() const;
	FVector GetBottomPoleWorldLocation() const;

	const UStaticMeshComponent* GetCableComponent() const { return CableStaticMeshComponent; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* FirstPoleStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* SecondPoleStaticMeshComponent;

	/*
	 * Z-Oriented. Origin must be at the edge of the cable cylinder, not at geometrical center
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* CableStaticMeshComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (MakeEditWidget))
	FVector FirstPoleLocation;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (MakeEditWidget))
	FVector SecondPoleLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float InteractionCapsuleRadius = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CableFriction = 100.0f;

	virtual void BeginPlay() override;

private:
	FVector AdjustedFirstPoleLocation;
	FVector AdjustedSecondPoleLocation;
	float FirstPoleHalfHeight;
	float SecondPoleHalfHeight;

	void Initialize();
};
