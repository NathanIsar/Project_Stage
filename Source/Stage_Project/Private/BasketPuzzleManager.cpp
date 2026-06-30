#include "BasketPuzzleManager.h"
#include "BasketMount.h"

ABasketPuzzleManager::ABasketPuzzleManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

bool ABasketPuzzleManager::IsAnyBasketRotating() const
{
	for (const ABasketMount* Basket : ManagedBaskets)
	{
		if (Basket && Basket->IsRotating())
		{
			return true;
		}
	}
	return false;
}

void ABasketPuzzleManager::ApplyRotationToAll(const FRotator& AxisDelta)
{
	if (IsAnyBasketRotating())
	{
		return;
	}

	for (ABasketMount* Basket : ManagedBaskets)
	{
		if (Basket)
		{
			Basket->AddRotationDelta(AxisDelta);
		}
	}
}