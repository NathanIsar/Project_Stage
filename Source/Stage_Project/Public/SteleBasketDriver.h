#pragma once

#include "CoreMinimal.h"
#include "SteleActor.h"
#include "SteleBasketDriver.generated.h"


UCLASS(Abstract, Blueprintable)
class STAGE_PROJECT_API ASteleBasketDriver : public ASteleActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void HandleSteleRotated(ESymbolType NewFrontSymbol);
};