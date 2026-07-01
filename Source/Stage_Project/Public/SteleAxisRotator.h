// SteleAxisRotator.h
#pragma once

#include "CoreMinimal.h"
#include "SteleBasketDriver.h"
#include "SteleAxisRotator.generated.h"

class ABasketPuzzleManager;

UCLASS(Blueprintable)
class STAGE_PROJECT_API ASteleAxisRotator : public ASteleBasketDriver
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|AxisRotator")
	TObjectPtr<ABasketPuzzleManager> PuzzleManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|AxisRotator")
	FRotator AxisDelta = FRotator(0.0f, 90.0f, 0.0f);

protected:
	virtual void OnBasketTrigger() override;
};