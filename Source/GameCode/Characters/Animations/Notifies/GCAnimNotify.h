#pragma once

#include "GCAnimNotify.generated.h"

UCLASS(Abstract)
class GAMECODE_API UGCAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
protected:
	class AGCBaseCharacter* GetCharacter(AActor* Actor) const;	
};
