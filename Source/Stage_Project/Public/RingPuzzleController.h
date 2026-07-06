#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RingPuzzleController.generated.h"

class AAlignmentRing;
class APuzzleDoor;

UCLASS()
class STAGE_PROJECT_API ARingPuzzleController : public AActor
{
	GENERATED_BODY()

public:
	ARingPuzzleController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, Category = "Puzzle")
	TArray<AAlignmentRing*> Rings;

	UPROPERTY(EditInstanceOnly, Category = "Puzzle")
	APuzzleDoor* Door = nullptr;

	UFUNCTION()
	void HandleRingAligned(AAlignmentRing* Ring);

private:
	int32 SolvedCount = 0;
};