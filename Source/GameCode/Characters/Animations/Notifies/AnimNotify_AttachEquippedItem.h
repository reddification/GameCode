// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCAnimNotify.h"
#include "AnimNotify_AttachEquippedItem.generated.h"

UCLASS()
class GAMECODE_API UAnimNotify_AttachEquippedItem : public UGCAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
