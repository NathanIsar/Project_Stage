#include "SteleBasketDriver.h"

void ASteleBasketDriver::Interact_Implementation(AActor* Interactor)
{
	Super::Interact_Implementation(Interactor);

	OnBasketTrigger();
}

void ASteleBasketDriver::OnBasketTrigger()
{
	// implémenté par les sous-classes.
}