// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_SetMeleeHitRegEnabled.h"

#include "Characters/GCBaseCharacter.h"
#include "Components/Character/CharacterEquipmentComponent.h"

void UAnimNotify_SetMeleeHitRegEnabled::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	auto Character = GetCharacter(MeshComp->GetOwner());
	if (IsValid(Character))
	{
		Character->GetEquipmentComponent()->SetMeleeHitRegEnabled(bEnabled);
	}
}
