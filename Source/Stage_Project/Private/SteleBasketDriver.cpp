// SteleBasketDriver.cpp
#include "SteleBasketDriver.h"

void ASteleBasketDriver::BeginPlay()
{
	Super::BeginPlay();

	OnSteleRotated.AddDynamic(this, &ASteleBasketDriver::HandleSteleRotated);
}

void ASteleBasketDriver::HandleSteleRotated(ESymbolType NewFrontSymbol)
{
	// laisse vide
}