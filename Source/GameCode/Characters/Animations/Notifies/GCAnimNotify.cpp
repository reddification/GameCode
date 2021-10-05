#include "GCAnimNotify.h"

#include "Characters/GCBaseCharacter.h"

AGCBaseCharacter* UGCAnimNotify::GetCharacter(AActor* Actor) const
{
	AGCBaseCharacter* Character = Cast<AGCBaseCharacter>(Actor);
	if (!IsValid(Character))
	{
		return nullptr;
	}

	return Character;
}
