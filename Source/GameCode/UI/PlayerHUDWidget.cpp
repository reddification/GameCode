#include "UI/PlayerHUDWidget.h"
#include "WeaponInfoWidget.h"
#include "ReticleWidget.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::OnAimingStateChanged(bool bAiming, EReticleType ReticleType)
{
	Reticle->OnAimingStateChanged(bAiming);
	Reticle->SetReticleType(ReticleType);
}

void UPlayerHUDWidget::SetAmmo(int32 ClipAmmo, int32 RemainingAmmo)
{
	if (WeaponInfoWidget->Visibility == ESlateVisibility::Hidden)
	{
		WeaponInfoWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	
	WeaponInfoWidget->SetAmmo(ClipAmmo, RemainingAmmo);
}

void UPlayerHUDWidget::SetThrowablesCount(int32 Count)
{
	WeaponInfoWidget->SetThrowablesCount(Count);
}

void UPlayerHUDWidget::OnAttributeChanged(ECharacterAttribute Attribute, float Value)
{
	CharacterAttributesWidget->SetAttribute(Attribute, Value);
}

void UPlayerHUDWidget::SetReticleType(EReticleType ReticleType)
{
	Reticle->SetReticleType(ReticleType);
}

void UPlayerHUDWidget::OnMeleeWeaponEquipped()
{
	Reticle->SetReticleType(EReticleType::None);
	WeaponInfoWidget->OnMeleeWeaponEquipped();
}

void UPlayerHUDWidget::SetWeaponName(const FText& Name)
{
	WeaponInfoWidget->ActiveWeaponNameTextblock->SetText(Name);
}

void UPlayerHUDWidget::SetThrowableName(const FText& Name)
{
	WeaponInfoWidget->ActiveThrowableNameTextblock->SetText(Name);
}
