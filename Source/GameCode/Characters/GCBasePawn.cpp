// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBasePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"
#include "GameCode/Components/Movement/GCBasePawnMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameCode/GameCode.h"
#include "GameFramework/SpringArmComponent.h"

AGCBasePawn::AGCBasePawn()
{
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetSphereRadius(CollisionSphereRadius);
	CollisionComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = CollisionComponent;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->bUsePawnControlRotation = 1;
	SpringArmComponent->SetupAttachment(RootComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->SetRelativeLocation(FVector(0, 0, 50));
	CameraComponent->SetRelativeRotation(FRotator(-20, 0, 0));

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	ArrowComponent->SetupAttachment(RootComponent);
#endif
}

// Called to bind functionality to input
void AGCBasePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("MoveForward", this, &AGCBasePawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGCBasePawn::MoveRight);
	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &AGCBasePawn::Jump);
}

void AGCBasePawn::MoveForward(float val)
{
	InputForward = val;
	if (val != 0.0f)
	{
		// AddMovementInput(CurrentViewActor->GetActorForwardVector(), val);
		FVector direction = CurrentViewCameraComponent == nullptr
			? GetActorForwardVector()
			: CurrentViewCameraComponent->GetForwardVector();
		
		AddMovementInput(direction, val);
	}
}

void AGCBasePawn::MoveRight(float val)
{
	InputRight = val;
	if (val != 0.0f)
	{
		// AddMovementInput(CurrentViewActor->GetActorRightVector(), val);
		FVector direction = CurrentViewCameraComponent == nullptr
			? GetActorRightVector()
			: CurrentViewCameraComponent->GetRightVector();
		AddMovementInput(direction, val);
	}
}

void AGCBasePawn::Jump()
{
	//why not make the component field itself of this type?
  	checkf(MovementComponent->IsA<UGCBasePawnMovementComponent>(),
  		TEXT("Only UGCBasePawnMovementComponent supported for JUMP"));
	UGCBasePawnMovementComponent* GCMovementComponent = static_cast<UGCBasePawnMovementComponent*>(MovementComponent);
	GCMovementComponent->JumpStart();
}

void AGCBasePawn::BeginPlay()
{
	Super::BeginPlay();
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	CurrentViewActor = CameraManager->GetViewTarget();
	CameraManager->OnBlendComplete().AddUFunction(this, FName("OnBlendComplete"));
}

void AGCBasePawn::OnBlendComplete()
{
	CurrentViewActor = GetController()->GetViewTarget();
	auto viewActorCameraComponent = CurrentViewActor->GetComponentByClass(TSubclassOf<UCameraComponent>());
	CurrentViewCameraComponent = viewActorCameraComponent != nullptr
		? static_cast<UCameraComponent*>(viewActorCameraComponent)
		: nullptr;
	
	UE_LOG(LogCameras, Verbose, TEXT("AGameCodeBasepawn::OnBlendComplete. Current view actor: %s"),
		*CurrentViewActor->GetName());
}



