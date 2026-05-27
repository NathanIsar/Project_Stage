// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "NumberGrab.h"
#include "Components/AudioComponent.h"
#include "PuzzleSlotComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STAGE_PROJECT_API UPuzzleSlotComponent :  public UStaticMeshComponent
{
	GENERATED_BODY()

public:	
	UPuzzleSlotComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle|Slot")
	int32 SlotIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Puzzle|Slot")
	float SnapRadius = 60.f;
	
	UPROPERTY(BlueprintReadOnly, Category="Puzzle|Slot")
	ANumberGrab* PlacedDigit = nullptr;
 
	bool IsEmpty() const { return PlacedDigit == nullptr; }
	
	UFUNCTION(BlueprintCallable, Category="Puzzle")
	bool TryPlaceDigit(ANumberGrab* Digit);
	
	UFUNCTION(BlueprintCallable, Category="Puzzle")
	void EjectDigit();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Puzzle")
	int32 GetPlacedValue() const;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotChanged, UPuzzleSlotComponent*, Slot);
	
	UPROPERTY(BlueprintAssignable, Category="Puzzle|Events")
	FOnSlotChanged OnSlotChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Sound")
	USoundBase* PlaceSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Sound")
	USoundBase* EjectSound = nullptr;
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Puzzle|Sound")
	UAudioComponent* SlotAudioComponent = nullptr;
	
};
