// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_ThrowingItem.h"

#include "Characters/GCBaseCharacter.h"
#include "Components/Character/CharacterEquipmentComponent.h"

void UAnimNotify_ThrowingItem::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	auto Character = GetCharacter(MeshComp->GetOwner());
	if (!Character) return;

	switch (ThrowNotifyType)
	{
		case EThrowNotifyType::Grab:
			Character->GetEquipmentComponent()->GrabThrowableItem();
			break;
		case EThrowNotifyType::Activate:
			Character->GetEquipmentComponent()->ActivateThrowableItem();
			break;
		case EThrowNotifyType::Release:
			Character->GetEquipmentComponent()->ReleaseThrowableItem();
			break;
		default:
			break;
	}
}
