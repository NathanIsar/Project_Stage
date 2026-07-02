#include "SteleGearDriver.h"
#include "BasketMount.h"

void ASteleGearDriver::OnBasketTrigger()
{
	if (DrivenBasket)
	{
		TArray<ABasketMount*> VisitedBaskets;
		DrivenBasket->PropagateRotation(RotationStep, VisitedBaskets);
	}
}
