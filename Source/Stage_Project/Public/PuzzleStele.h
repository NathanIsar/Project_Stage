// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzleSlotComponent.h"
#include "Sound/SoundBase.h"
#include "PuzzleStele.generated.h"


class APuzzleDoor;

UENUM(BlueprintType)
enum class EPuzzleResult : uint8
{
	Correct,
	Wrong,
	Incomplete   
};

UCLASS()
class STAGE_PROJECT_API APuzzleStele : public AActor
{
	GENERATED_BODY()
	
public:	
	APuzzleStele();
	
	// ─── Config ───────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Code")
	TArray<int32> SecretCode;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Puzzle|Code")
	TArray<UPuzzleSlotComponent*> Slots;
 
	// ─── Actions ──────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category="Puzzle")
	EPuzzleResult CheckCode();
	
	UFUNCTION(BlueprintCallable, Category="Puzzle")
	bool TrySnapDigitToNearestSlot(ANumberGrab* Digit);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleEvent);
	
	UPROPERTY(BlueprintAssignable, Category="Puzzle|Events")
	FOnPuzzleEvent OnCodeValidated;
	
	UPROPERTY(BlueprintAssignable, Category="Puzzle|Events")
	FOnPuzzleEvent OnCodeWrong;
	
	UPROPERTY(EditAnywhere, Category="Puzzle")
	APuzzleDoor* LinkedDoor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle|FX")
	USoundBase* SuccessSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle|FX")
	USoundBase* FailSound = nullptr;

protected:

	virtual void BeginPlay() override;

private:
	
	void InitSlots();
 
	UFUNCTION()
	void HandleSlotChanged(UPuzzleSlotComponent* Slot);
};
