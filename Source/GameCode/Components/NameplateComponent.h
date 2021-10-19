// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NameplateComponent.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API UNameplateComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	const FText& GetName() const { return Name; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;
};
