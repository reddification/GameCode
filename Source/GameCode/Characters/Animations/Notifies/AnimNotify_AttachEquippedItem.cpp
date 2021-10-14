// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_AttachEquippedItem.h"

#include "Characters/GCBaseCharacter.h"
#include "Components/Character/CharacterEquipmentComponent.h"

void UAnimNotify_AttachEquippedItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	auto Character = GetCharacter(MeshComp->GetOwner());
	if (Character)
		Character->GetEquipmentComponent()->OnWeaponsChanged();
}
