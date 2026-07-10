#include "FractureFeedback.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

AFractureFeedback::AFractureFeedback()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFractureFeedback::BeginPlay()
{
	Super::BeginPlay();

	if (!TargetWall)
	{
		UE_LOG(LogTemp, Warning, TEXT("FractureFeedback: TargetWall non assigne!"));
		return;
	}
	
	UGeometryCollectionComponent* GC =
		TargetWall->FindComponentByClass<UGeometryCollectionComponent>();

	if (!GC)
	{
		UE_LOG(LogTemp, Warning, TEXT("FractureFeedback: pas de GeometryCollectionComponent sur %s"),
			*TargetWall->GetName());
		return;
	}
	
	GC->SetNotifyBreaks(true);
	GC->OnChaosBreakEvent.AddDynamic(this, &AFractureFeedback::OnWallBreak);
	
	if (AutoArmDelay > 0.0f)
    	{
    		GetWorldTimerManager().SetTimer(
    			ArmTimerHandle, this, &AFractureFeedback::ArmFeedback, AutoArmDelay, false);
    	}
}

void AFractureFeedback::ArmFeedback()
{
	bArmed = true;
}

void AFractureFeedback::OnWallBreak(const FChaosBreakEvent& BreakEvent)
{

	if (!bArmed)
		return; 
		
	if (bHasTriggered)
		return;
	
	if (BreakEvent.Mass < MinBreakMass)
		return;

	bHasTriggered = true;

	if (BreakSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BreakSound, BreakEvent.Location);
	}

	if (BreakCameraShakeClass)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientStartCameraShake(BreakCameraShakeClass, CameraShakeScale);
		}
	}
}