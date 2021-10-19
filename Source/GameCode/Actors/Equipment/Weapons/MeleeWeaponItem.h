#pragma once

#include "CoreMinimal.h"
#include "Actors/Equipment/EquippableItem.h"
#include "Data/MeleeAttackData.h"
#include "MeleeWeaponItem.generated.h"

UCLASS(Blueprintable)
class GAMECODE_API AMeleeWeaponItem : public AEquippableItem
{
	GENERATED_BODY()

public:
	AMeleeWeaponItem();
	
	const FMeleeAttackData* GetMeleeAttackData(EMeleeAttackType AttackType) { return Attacks.Find(AttackType); }

	void SetIsHitRegistrationEnabled(bool bEnabled);
	void StartAttack(EMeleeAttackType AttackType, AController* AttackerController);
	void StopAttack();
	const FMeleeAttackData* GetActiveAttack() const { return ActiveAttack; }
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EMeleeAttackType, FMeleeAttackData> Attacks;

private:

	void OnMeleeHitRegistered(const FHitResult& HitResult, const FVector& Direction);
	
	TArray<class UMeleeHitRegistratorComponent*> HitRegistrators;
	TSet<AActor*> HitActors;

	const FMeleeAttackData* ActiveAttack = nullptr;
	AController* AttackerController;

};
