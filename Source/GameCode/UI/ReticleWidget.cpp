// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ReticleWidget.h"

#include "Components/CanvasPanel.h"

void UReticleWidget::SetReticleType(EReticleType NewReticleType)
{
	ReticleType = NewReticleType;
	switch (ReticleType)
	{
	case EReticleType::Crosshair:
		CrosshairCanvas->SetVisibility(ESlateVisibility::HitTestInvisible);
		ScopeCanvas->SetVisibility(ESlateVisibility::Hidden);
		break;
	case EReticleType::Scope:
		CrosshairCanvas->SetVisibility(ESlateVisibility::Hidden);
		ScopeCanvas->SetVisibility(ESlateVisibility::HitTestInvisible);
		break;
	case EReticleType::None:
	default:
		CrosshairCanvas->SetVisibility(ESlateVisibility::Hidden);
		ScopeCanvas->SetVisibility(ESlateVisibility::Hidden);
		break;
	}
}
