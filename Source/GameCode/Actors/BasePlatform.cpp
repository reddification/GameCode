// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlatform.h"

#include "PlatformInvocator.h"

ABasePlatform::ABasePlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("root"));
	RootComponent = DefaultPlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform mesh"));
	PlatformMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	EndLocation = GetActorLocation() + GetActorRotation().RotateVector(EndLocation);
	
	if (IsValid(PlatformTimelineCurve))
	{
		FOnTimelineFloatStatic PlatformTimelineCallback;
		PlatformTimelineCallback.BindUObject(this, &ABasePlatform::SetPlatformPosition);
		PlatformTimeline.AddInterpFloat(PlatformTimelineCurve, PlatformTimelineCallback);

		FOnTimelineEventStatic OnTimelineEvent;
		OnTimelineEvent.BindUObject(this, &ABasePlatform::OnPlatformReachedFinalPosition);
		PlatformTimeline.SetTimelineFinishedFunc(OnTimelineEvent);
		
		if (IsValid(PlatformInvocator) && PlatformBehavior == EPlatformBehavior::ActivatedByGenerator)
		{
			PlatformInvocator->OnInvocatorActivated.AddDynamic(this, &ABasePlatform::OnPlatformInvoked);
		}

		if (bActive)
        {
            PlatformTimeline.Play();
        }
	}
}

void ABasePlatform::OnPlatformInvoked()
{
	if (!PlatformTimeline.IsPlaying() || PlatformTimeline.IsReversing())
	{
		bReverse = false;
		PlatformTimeline.Play();
	}
}

void ABasePlatform::Start()
{
	if (!PlatformTimeline.IsPlaying())
	{
		bReverse ? PlatformTimeline.Reverse() : PlatformTimeline.Play();
	}
}

void ABasePlatform::ReverseStop()
{
	bReverse = !bReverse;	
	PlatformTimeline.Stop();
}

// Called every frame
void ABasePlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PlatformTimeline.IsPlaying())
	{
		PlatformTimeline.TickTimeline(DeltaTime);
	}
}

void ABasePlatform::ResetPlatform()
{
	bReverse = true;
	GetWorld()->GetTimerManager().ClearTimer(this->PlatformReturnDelayTimer);
	PlatformTimeline.Reverse();
}

void ABasePlatform::ResumePlatform()
{
	bReverse = !bReverse;
	bReverse ? PlatformTimeline.Reverse() : PlatformTimeline.Play();
	GetWorld()->GetTimerManager().ClearTimer( PlatformLoopDelayTimer);
}

void ABasePlatform::SetPlatformPosition(float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	SetActorLocation(PlatformTargetLocation);
}

void ABasePlatform::OnPlatformReachedFinalPosition()
{
	switch (PlatformBehavior)
	{
		case EPlatformBehavior::ActivatedByGenerator:
			if (!bReverse)
			{
				GetWorld()->GetTimerManager().SetTimer(PlatformReturnDelayTimer, this, &ABasePlatform::ResetPlatform, PlatformResetDelay, false);
			}
			break;
		case EPlatformBehavior::Loop:
			GetWorld()->GetTimerManager().SetTimer(PlatformLoopDelayTimer, this, &ABasePlatform::ResumePlatform, PlatformLoopDelay, false);
			break;
		default:
			break;
	}
}
