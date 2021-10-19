// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WeaponInfoWidget.h"

#include "Components/TextBlock.h"

void UWeaponInfoWidget::SetAmmo(int32 CurrentAmmo, int32 TotalAmmo)
{
	ClipAmmoTextblock->SetText(FText::AsNumber(CurrentAmmo));
	RemainingAmmoTextblock->SetText(FText::AsNumber(TotalAmmo));
	if (AmmoWidgetsContainerWidget->Visibility == ESlateVisibility::Hidden)
	{
		AmmoWidgetsContainerWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UWeaponInfoWidget::SetThrowablesCount(int32 ThrowablesCount)
{
	RemainingThrowablesTextblock->SetText(FText::AsNumber(ThrowablesCount));
}

void UWeaponInfoWidget::OnMeleeWeaponEquipped()
{
	AmmoWidgetsContainerWidget->SetVisibility(ESlateVisibility::Hidden);
}
