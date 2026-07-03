#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StatuePuzzleController.generated.h"

class ARotatingStatueActor;
class APuzzleDoor;

UCLASS()
class STAGE_PROJECT_API AStatuePuzzleController : public AActor
{
	GENERATED_BODY()

public:
	AStatuePuzzleController();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle")
	TArray<ARotatingStatueActor*> Statues;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Puzzle")
	APuzzleDoor* DoorToUnlock;

private:
	bool bIsSolved = false;

	UFUNCTION()
	void OnAnyStatueOrientationChanged(bool bIsCorrectOrientation);

	void CheckPuzzleSolved();
};