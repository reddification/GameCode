#pragma once

#include "CoreMinimal.h"
#include "Characters/Animations/Notifies/GCAnimNotify.h"
#include "AnimNotify_SetMeleeHitRegEnabled.generated.h"

UCLASS()
class GAMECODE_API UAnimNotify_SetMeleeHitRegEnabled : public UGCAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEnabled = true;
};
