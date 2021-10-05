// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CharacterAttributesWidget.h"
#include "Components/ProgressBar.h"

void UCharacterAttributesWidget::SetAttribute(ECharacterAttribute Attribute, float Value)
{
	UProgressBar* Bar = nullptr;
	switch (Attribute)
	{
		case ECharacterAttribute::Health:
			Bar = HealthBar;
			break;
		case ECharacterAttribute::Stamina:
			Bar = StaminaBar;
			break;
		case ECharacterAttribute::Oxygen:
			Bar = OxygenBar;
			break;
		default:
			break;
	}

	if (!IsValid(Bar))
	{
		return;
	}
	
	if (FMath::IsNearlyEqual(Bar->Percent, 1.f, 0.01f) && Value < Bar->Percent)
	{
		OnAttributeChanged(Attribute);	
	}
	else if (FMath::IsNearlyEqual(Value, 1.f, 0.01f) && Value > Bar->Percent)
	{
		OnAttributeRestored(Attribute);
	}

	Bar->SetPercent(Value);
}
