#include "RingPuzzleController.h"
#include "AlignmentRing.h"
#include "PuzzleDoor.h"

ARingPuzzleController::ARingPuzzleController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARingPuzzleController::BeginPlay()
{
	Super::BeginPlay();

	for (AAlignmentRing* Ring : Rings)
	{
		if (Ring)
		{
			Ring->OnRingAligned.AddDynamic(this, &ARingPuzzleController::HandleRingAligned);
		}
	}
}

void ARingPuzzleController::HandleRingAligned(AAlignmentRing* /*Ring*/)
{
	++SolvedCount;
	if (SolvedCount >= Rings.Num() && Door)
	{
		Door->Open(); 
	}
}