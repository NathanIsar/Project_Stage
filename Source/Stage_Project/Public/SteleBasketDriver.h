// SteleBasketDriver.h
#pragma once

#include "CoreMinimal.h"
#include "BaseInteractableActor.h"
#include "SteleBasketDriver.generated.h"

UCLASS(Abstract, Blueprintable)
class STAGE_PROJECT_API ASteleBasketDriver : public ABaseInteractableActor
{
	GENERATED_BODY()

public:
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	virtual void OnBasketTrigger();
};