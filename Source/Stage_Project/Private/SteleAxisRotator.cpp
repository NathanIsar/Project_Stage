#include "SteleAxisRotator.h"
#include "BasketPuzzleManager.h"

void ASteleAxisRotator::HandleSteleRotated(ESymbolType NewFrontSymbol)
{
	if (PuzzleManager)
	{
		PuzzleManager->ApplyRotationToAll(AxisDelta);
	}
}