// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractiveActor.generated.h"

UENUM()
enum class EInteractionType : uint8
{
	None = 0 UMETA(DisplayName="None"),
	Climbing = 1 UMETA(DisplayName="Climbing"),
	Ziplining = 2 UMETA(DisplayName="Ziplining"),
};

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AInteractiveActor : public AActor
{
	GENERATED_BODY()

public:
	virtual EInteractionType GetInteractionType() const { return EInteractionType::None; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Interaction")
	class UPrimitiveComponent* InteractionVolume;

	UFUNCTION()
	virtual void OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	virtual void OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsOverlappingCharacterCapsule(const class ACharacter* Character, const UPrimitiveComponent* OtherComp) const;
};
