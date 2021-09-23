// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AmmoWidget.h"

#include "Components/TextBlock.h"

void UAmmoWidget::SetAmmo(int32 CurrentAmmo, int32 TotalAmmo)
{
	ClipAmmoTextblock->SetText(FText::AsNumber(CurrentAmmo));
	RemainingAmmoTextblock->SetText(FText::AsNumber(TotalAmmo));
}