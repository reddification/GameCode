// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Animations/Notifies/AnimNotify_ReloadShellInserted.h"

#include "Characters/GCBaseCharacter.h"
#include "Components/CharacterEquipmentComponent.h"

void UAnimNotify_ReloadShellInserted::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(Character))
	{
		return;
	}

	Character->GetEquipmentComponent()->ReloadInsertShells(ShellsInsertedAtOnce);
}
