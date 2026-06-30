// BasketPuzzleManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasketPuzzleManager.generated.h"

class ABasketMount;

UCLASS(Blueprintable)
class STAGE_PROJECT_API ABasketPuzzleManager : public AActor
{
	GENERATED_BODY()

public:
	ABasketPuzzleManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle")
	TArray<TObjectPtr<ABasketMount>> ManagedBaskets;
	
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void ApplyRotationToAll(const FRotator& AxisDelta);

	UFUNCTION(BlueprintPure, Category = "Puzzle")
	bool IsAnyBasketRotating() const;
};