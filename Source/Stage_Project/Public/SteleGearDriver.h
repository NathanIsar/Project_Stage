#pragma once

#include "CoreMinimal.h"
#include "SteleBasketDriver.h"
#include "SteleGearDriver.generated.h"

class ABasketMount;

UCLASS(Blueprintable)
class STAGE_PROJECT_API ASteleGearDriver : public ASteleBasketDriver
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|GearDriver")
	TObjectPtr<ABasketMount> DrivenBasket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|GearDriver")
	FRotator RotationStep = FRotator(0.0f, 90.0f, 0.0f);

protected:
	virtual void OnBasketTrigger() override;
};