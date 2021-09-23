// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_AttachEquippedItem.h"

#include "Characters/GCBaseCharacter.h"
#include "Components/CharacterEquipmentComponent.h"

void UAnimNotify_AttachEquippedItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AGCBaseCharacter* Character =  Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(Character))
	{
		return;
	}

	Character->GetEquipmentComponent()->OnItemsChanged();
}
