// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerHUDWidget.h"

#include "AmmoWidget.h"
#include "ReticleWidget.h"
#include "Components/ProgressBar.h"

void UPlayerHUDWidget::SetHealth(float Value)
{
	Healthbar->SetPercent(Value);
}

void UPlayerHUDWidget::OnAimingStateChanged(bool bAiming)
{
	Reticle->OnAimingStateChanged(bAiming);
}

void UPlayerHUDWidget::SetAmmo(int32 ClipAmmo, int32 RemainingAmmo)
{
	AmmoWidget->SetAmmo(ClipAmmo, RemainingAmmo);
}
