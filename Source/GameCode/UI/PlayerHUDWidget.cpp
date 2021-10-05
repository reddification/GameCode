// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerHUDWidget.h"
#include "AmmoWidget.h"
#include "ReticleWidget.h"

void UPlayerHUDWidget::OnAimingStateChanged(bool bAiming, EReticleType ReticleType)
{
	Reticle->OnAimingStateChanged(bAiming);
	Reticle->SetReticleType(ReticleType);
}

void UPlayerHUDWidget::SetAmmo(int32 ClipAmmo, int32 RemainingAmmo)
{
	AmmoWidget->SetAmmo(ClipAmmo, RemainingAmmo);
}

void UPlayerHUDWidget::OnAttributeChanged(ECharacterAttribute Attribute, float Value)
{
	CharacterAttributesWidget->SetAttribute(Attribute, Value);
}

void UPlayerHUDWidget::SetReticleType(EReticleType ReticleType)
{
	Reticle->SetReticleType(ReticleType);
}
