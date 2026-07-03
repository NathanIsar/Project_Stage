#include "StatuePuzzleController.h"
#include "RotatingStatueActor.h"
#include "PuzzleDoor.h"

AStatuePuzzleController::AStatuePuzzleController()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AStatuePuzzleController::BeginPlay()
{
	Super::BeginPlay();

	for (ARotatingStatueActor* Statue : Statues)
	{
		if (Statue)
		{
			Statue->OnOrientationChanged.AddDynamic(this, &AStatuePuzzleController::OnAnyStatueOrientationChanged);
		}
	}
}

void AStatuePuzzleController::OnAnyStatueOrientationChanged(bool bIsCorrectOrientation)
{
	CheckPuzzleSolved();
}

void AStatuePuzzleController::CheckPuzzleSolved()
{
	if (bIsSolved) return;

	for (ARotatingStatueActor* Statue : Statues)
	{
		if (!Statue || !Statue->IsCorrectOrientation())
		{
			return;
		}
	}

	bIsSolved = true;

	if (DoorToUnlock)
	{
		DoorToUnlock->Open();
	}
}