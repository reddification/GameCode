// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameCode/Data/IKData.h"
#include "GameCode/Data/IKSettings.h"

#include "InverseKinematicsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UInverseKinematicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	void CalculateIkData(const USkeletalMeshComponent* SkeletalMesh, float CapsuleHalfHeight, FVector ActorLocation,
		bool bCrouched, float DeltaTime);

	const FIkData& GetIkData() const { return IkData; }
	void SetScale(float Scale) { IkData.IKScale = Scale; }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character|IK")
	FIKSettings IkSettings;
	
private:
	FIkData IkData;

	void RecalculateFeetPitches(const USkeletalMeshComponent* SkeletalMesh, float DeltaSeconds);
	void RecalculateKneesExtends(float DeltaSeconds);
	void UpdateLegsIkOffsetsBoxTrace(const USkeletalMeshComponent* SkeletalMesh, float CapsuleHalfHeight,
		const FVector& ActorLocation, bool bCrouched, float DeltaSeconds);
	void RecalculateFeetElevationsWithPelvis();
	float GetIkElevationForSocket(const FName& SocketName, const USkeletalMeshComponent* SkeletalMesh,
		const FVector& ActorLocation, float CapsuleHalfHeight, bool bCrouched);
	float CalculateFootPitch(const USkeletalMeshComponent* SkeletalMesh, const FName& HeelSocketName, const FName& FootSocketName,
		const FName& ToesSocketName, float PreviousValue);
};
