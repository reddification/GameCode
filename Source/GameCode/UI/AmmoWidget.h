// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoWidget.generated.h"

class UTextBlock;

UCLASS()
class GAMECODE_API UAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetAmmo(int32 CurrentAmmo, int32 TotalAmmo);
	
protected:
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* ClipAmmoTextblock;
	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* RemainingAmmoTextblock;
};
