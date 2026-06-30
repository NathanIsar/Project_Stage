#include "SteleGearDriver.h"
#include "BasketMount.h"

void ASteleGearDriver::HandleSteleRotated(ESymbolType NewFrontSymbol)
{
	if (DrivenBasket)
	{
		TArray<ABasketMount*> VisitedBaskets;
		DrivenBasket->PropagateRotation(RotationStep, VisitedBaskets);
	}
}