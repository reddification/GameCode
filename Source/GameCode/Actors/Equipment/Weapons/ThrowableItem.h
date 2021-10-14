#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "Components/CharacterEquipmentComponent.h"
#include "ThrowableItem.generated.h"

class AGCProjectile;
UCLASS(Blueprintable)
class GAMECODE_API AThrowableItem : public AEquippableItem
{
	GENERATED_BODY()

public:
	void Throw(AController* OwnerController);

	const TSubclassOf<AGCProjectile>& GetProjectileClass() const { return ProjectileClass; }
	UAnimMontage* GetThrowMontage() const { return ThrowMontage; }
	void Activate(AController* Controller);
	float GetThrowDuration() const { return ThrowDuration; }
	AGCProjectile* GetCurrentProjectile();
	AGCProjectile* SpawnProjectile();
	void DropProjectile(AController* Controller);
	
	EAmmunitionType GetAmmunitionType() const { return AmmunitionType; }
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AGCProjectile> ProjectileClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EAmmunitionType AmmunitionType = EAmmunitionType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables", meta=(UIMin=0.f, ClampMin=0.f))
	float ThrowDuration = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables")
	UAnimMontage* ThrowMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables", meta=(UIMin=-90.f, ClampMin=-90.f, UIMax=90.f, ClampMax=90.f))
	float ThrowAngle = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throwables", meta=(UIMin=10.f, ClampMin=10.f))
	float ThrowSpeed = 1500.f;

	virtual void BeginPlay() override;
	
private:
	TWeakObjectPtr<AGCProjectile> AttachedProjectile;
	TWeakObjectPtr<AGCProjectile> CurrentProjectile;
};
