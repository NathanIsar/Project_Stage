// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SteleActor.h"
#include "Stelepuzzlemanager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPuzzleSolved);


UCLASS()
class STAGE_PROJECT_API AStelepuzzlemanager : public AActor
{
	GENERATED_BODY()
	
public:
    AStelepuzzlemanager();

protected:
    virtual void BeginPlay() override;

    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Steles")
    ASteleActor* Stele1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Steles")
    ASteleActor* Stele2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Steles")
    ASteleActor* Stele3;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Solution")
    ESymbolType CorrectSymbol_Stele1 = ESymbolType::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Solution")
    ESymbolType CorrectSymbol_Stele2 = ESymbolType::Cylinder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Solution")
    ESymbolType CorrectSymbol_Stele3 = ESymbolType::Cube;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Door")
    class ABaseInteractableActor* DoorToUnlock;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Door")
    ESymbolType DoorSymbol1 = ESymbolType::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Door")
    ESymbolType DoorSymbol2 = ESymbolType::Cylinder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Door")
    ESymbolType DoorSymbol3 = ESymbolType::Cube;

    // === OPTIONS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Settings")
    bool bShowDebugMessages = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Settings")
    bool bResetOnFail = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Settings")
    float ResetDelay = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Settings")
    bool bPlaySoundOnSuccess = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Settings")
    bool bPlaySoundOnFail = true;

    // === FEEDBACK ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Feedback")
    class USoundBase* PuzzleSolvedSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puzzle|Feedback")
    class USoundBase* PuzzleFailedSound;
    
    // === DELEGATES ===

    UPROPERTY(BlueprintAssignable, Category = "Puzzle")
    FOnPuzzleSolved OnPuzzleSolved;

    // === BLUEPRINT EVENTS ===

    UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle")
    void OnPuzzleCompleted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle")
    void OnPuzzleFailed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Puzzle")
    void OnSteleChanged(int32 SteleIndex, ESymbolType NewSymbol);

    // === PUBLIC FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void CheckSolution();
    
    UFUNCTION(BlueprintCallable, Category = "Puzzle")
    void ResetPuzzle();
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Puzzle")
    bool IsPuzzleSolved() const { return bIsSolved; }
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Puzzle")
    void GetCurrentSymbols(ESymbolType& Symbol1, ESymbolType& Symbol2, ESymbolType& Symbol3) const;

private:
    bool bIsSolved = false;

    UFUNCTION()
    void OnStele1Rotated(ESymbolType NewSymbol);

    UFUNCTION()
    void OnStele2Rotated(ESymbolType NewSymbol);

    UFUNCTION()
    void OnStele3Rotated(ESymbolType NewSymbol);

    void BindSteleEvents();
    bool CheckIfSolutionCorrect() const;
    void UnlockDoor();
    void HandlePuzzleFailed();

};