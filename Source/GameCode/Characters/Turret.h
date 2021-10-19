#pragma once

#include "CoreMinimal.h"
#include "Data/AITypesGC.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/Killable.h"
#include "UObject/WeakInterfacePtr.h"
#include "Turret.generated.h"

UENUM(BlueprintType)
enum class ETurretMode : uint8
{
	Search,
	Attack
};

UCLASS()
class GAMECODE_API ATurret : public APawn
{
	GENERATED_BODY()

public:
	ATurret();

	virtual void Tick(float DeltaTime) override;

	void SetCurrentTarget(AActor* NewTarget);

	virtual FVector GetPawnViewLocation() const override;
	virtual FRotator GetViewRotation() const override;

	virtual void PossessedBy(AController* NewController) override;
	ETeam GetTeam() const { return Team; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* TurretBaseComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* TurretGunComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UTurretBarrelComponent* TurretBarrelComponent;

	// per sec
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, UIMin=0.f))
	float BaseSearchRotationRate = 60.f;

	// per sec
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, UIMin=0.f))
	float BarrelPitchRotationRate = 60.f;

	// per sec
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=-75.f, UIMin=-75.f))
	float MinBarrelPitchRotationRate = -75.f;

	// per sec
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=75.f, UIMin=75.f))
	float MaxBarrelPitchRotationRate = 75.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, UIMin=0.f))
	float LookAtInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=1.f, UIMin=1.f))
	float FireRate = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, UIMin=0.f))
	float BulletSpreadAngle = 1.f;

	// in seconds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.f, UIMin=0.f))
	float FireDelayTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETeam Team = ETeam::BadGuys;

private:
	void Search(float DeltaTime);
	void Track(float DeltaTime);
	void SetMode(ETurretMode NewMode);
	float GetFireInterval() const { return 60.f / FireRate; }

	void Shoot();
	
	ETurretMode CurrentMode = ETurretMode::Search;

	TWeakObjectPtr<AActor> Target = nullptr;
	IKillable* KillableTarget = nullptr;
	
	FTimerHandle ShootDelayTimer;
};
