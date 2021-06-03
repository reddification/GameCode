// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GCBasePawn.h"
#include "SpiderPawn.generated.h"

/**
 * 
 */
UCLASS()
class GAMECODE_API ASpiderPawn : public AGCBasePawn
{
	GENERATED_BODY()

public:
	ASpiderPawn();
	
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightFrontFootOffset() const { return IKRightFrontFootOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftFrontFootOffset() const { return IKLeftFrontFootOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKRightRearFootOffset() const { return IKRightRearFootOffset; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetIKLeftRearFootOffset() const { return IKLeftRearFootOffset; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spider bot")
	class USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName RightFrontFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName LeftFrontFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName LeftRearFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	FName RightRearFootSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spider bot|IK settings", meta=(ClampMin = 0, UIMin = 0))
	float IKTraceExtendDistance = 50;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spider bot|IK settings", meta=(ClampMin = 0, UIMin = 0))
	float IKInterpSpeed = 20;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Spider bot|IK settings")
	float IKHangingFootOffset = 35;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spider bot|IK settings")
	float MaxLegExtend = 50;
	
private:
	float IKRightFrontFootOffset = 0;
	float IKLeftFrontFootOffset = 0;
	float IKRightRearFootOffset = 0;
	float IKLeftRearFootOffset = 0;
	
	float IKTraceDistance = 0;
	float IKScale = 1;
	float GetIkOffsetForSocket(const FName& SocketName);
};
