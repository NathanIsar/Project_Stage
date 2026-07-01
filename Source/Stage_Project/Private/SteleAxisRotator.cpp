#include "SteleAxisRotator.h"
#include "BasketPuzzleManager.h"

void ASteleAxisRotator::OnBasketTrigger()
{
	if (PuzzleManager)
	{
		PuzzleManager->ApplyRotationToAll(AxisDelta);
	}
}