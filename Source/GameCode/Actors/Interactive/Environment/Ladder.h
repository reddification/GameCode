// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Ladder.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UAnimMontage;

UCLASS(Blueprintable)
class GAMECODE_API ALadder : public AInteractiveActor
{
	GENERATED_BODY()

public:
	ALadder();
	virtual void OnConstruction(const FTransform& Transform) override;
	float GetHeight() const { return Height; }
	bool IsOnTop() const { return bOnTop; }
	UAnimMontage* GetAttachFromTopAnimMontage() const { return AttachFromTopAnimMontage; }
	FVector GetAttachFromTopAnimMontageStartingLocation() const;
	virtual EInteractionType GetInteractionType() const override { return EInteractionType::Climbing; }

protected:
	virtual void BeginPlay() override;
	virtual void OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder", meta=(ClampMin=50.0f, UIMin=50.0f))
	float Height = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder", meta=(ClampMin=50.0f, UIMin=50.0f))
	float Width = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder", meta=(ClampMin=50.0f, UIMin=50.0f))
	float StepsInterval = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Ladder", meta=(ClampMin=50.0f, UIMin=50.0f))
	float FirstStepOffset = 25.0f;

	// TODO v2 with single vertical mesh reference and 2 instanced vertical mesh components
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* LeftRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* RightRailMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UInstancedStaticMeshComponent* StepMeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* TopInteractionVolume;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder")
	UAnimMontage* AttachFromTopAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ladder")
	FVector AttachFromTopAnimMontageInitialOffset = FVector::ZeroVector;
	
	UBoxComponent* GetLadderInteractionVolume() const;

private:
	void SetupLadderVerticalRail(UStaticMeshComponent* RailMeshComponent, float OffsetY);
	void SetupStep();

	bool bOnTop = false;
};
