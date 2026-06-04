// Fill out your copyright notice in the Description page of Project Settings.


#include "StelePuzzleManager.h"
#include "DoorActor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AStelepuzzlemanager::AStelepuzzlemanager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AStelepuzzlemanager::BeginPlay()
{
    Super::BeginPlay();
    
    if (!Stele1 || !Stele2 || !Stele3)
    {
        UE_LOG(LogTemp, Error, TEXT("StelePuzzleManager: Missing stele references!"));
        return;
    }

    if (!DoorToUnlock)
    {
        UE_LOG(LogTemp, Warning, TEXT("StelePuzzleManager: No door assigned!"));
    }
    
    BindSteleEvents();
    
    CheckSolution();
}

void AStelepuzzlemanager::BindSteleEvents()
{
    if (Stele1)
    {
        Stele1->OnSteleRotated.AddDynamic(this, &AStelepuzzlemanager::OnStele1Rotated);
    }

    if (Stele2)
    {
        Stele2->OnSteleRotated.AddDynamic(this, &AStelepuzzlemanager::OnStele2Rotated);
    }

    if (Stele3)
    {
        Stele3->OnSteleRotated.AddDynamic(this, &AStelepuzzlemanager::OnStele3Rotated);
    }
}

void AStelepuzzlemanager::OnStele1Rotated(ESymbolType NewSymbol)
{
    if (bShowDebugMessages)
    {
        UE_LOG(LogTemp, Log, TEXT("Stele 1 rotated to: %d"), static_cast<int32>(NewSymbol));
    }

    OnSteleChanged(1, NewSymbol);
    CheckSolution();
}

void AStelepuzzlemanager::OnStele2Rotated(ESymbolType NewSymbol)
{
    if (bShowDebugMessages)
    {
        UE_LOG(LogTemp, Log, TEXT("Stele 2 rotated to: %d"), static_cast<int32>(NewSymbol));
    }

    OnSteleChanged(2, NewSymbol);
    CheckSolution();
}

void AStelepuzzlemanager::OnStele3Rotated(ESymbolType NewSymbol)
{
    if (bShowDebugMessages)
    {
        UE_LOG(LogTemp, Log, TEXT("Stele 3 rotated to: %d"), static_cast<int32>(NewSymbol));
    }

    OnSteleChanged(3, NewSymbol);
    CheckSolution();
}

void AStelepuzzlemanager::CheckSolution()
{
    if (bIsSolved)
    {
        return;
    }

    bool bCorrect = CheckIfSolutionCorrect();

    if (bCorrect)
    {
        bIsSolved = true;

        if (bShowDebugMessages)
        {
            UE_LOG(LogTemp, Log, TEXT("PUZZLE RÉSOLU!"));
        }
        
        if (bPlaySoundOnSuccess && PuzzleSolvedSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), PuzzleSolvedSound);
        }
        
        UnlockDoor();
        
        OnPuzzleCompleted();
        OnPuzzleSolved.Broadcast();
    }
    else
    {
        if (bResetOnFail)
        {
            HandlePuzzleFailed();
        }
    }
}

bool AStelepuzzlemanager::CheckIfSolutionCorrect() const
{
    if (!Stele1 || !Stele2 || !Stele3)
        return false;

    ESymbolType Current1 = Stele1->GetCurrentFrontSymbol();
    ESymbolType Current2 = Stele2->GetCurrentFrontSymbol();
    ESymbolType Current3 = Stele3->GetCurrentFrontSymbol();

    bool bMatch1 = (Current1 == CorrectSymbol_Stele1);
    bool bMatch2 = (Current2 == CorrectSymbol_Stele2);
    bool bMatch3 = (Current3 == CorrectSymbol_Stele3);

    if (bShowDebugMessages)
    {
        UE_LOG(LogTemp, Log, TEXT("Solution check - Stele1: %s | Stele2: %s | Stele3: %s"),
            bMatch1 ? TEXT("✓") : TEXT("✗"),
            bMatch2 ? TEXT("✓") : TEXT("✗"),
            bMatch3 ? TEXT("✓") : TEXT("✗"));
    }

    return bMatch1 && bMatch2 && bMatch3;
}

void AStelepuzzlemanager::UnlockDoor()
{
    if (DoorToUnlock)
    {
        ADoorActor* Door = Cast<ADoorActor>(DoorToUnlock);
        
        if (Door)
        {
            Door->UnlockAndOpen();
            
            if (bShowDebugMessages)
            {
                UE_LOG(LogTemp, Log, TEXT("Door unlocked and opened!"));
            }
        }
        else
        {
            
            if (bShowDebugMessages)
            {
                UE_LOG(LogTemp, Warning, TEXT("Door unlocked (not a DoorActor)"));
            }
        }
    }
}

void AStelepuzzlemanager::HandlePuzzleFailed()
{
    if (bPlaySoundOnFail && PuzzleFailedSound)
    {
        UGameplayStatics::PlaySound2D(GetWorld(), PuzzleFailedSound);
    }

    OnPuzzleFailed();
    
    FTimerHandle ResetTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        ResetTimerHandle,
        this,
        &AStelepuzzlemanager::ResetPuzzle,
        ResetDelay,
        false
    );

    if (bShowDebugMessages)
    {
        UE_LOG(LogTemp, Warning, TEXT("Puzzle failed - resetting in %.1f seconds"), ResetDelay);
    }
}

void AStelepuzzlemanager::ResetPuzzle()
{
    if (Stele1)
        Stele1->ResetStele();

    if (Stele2)
        Stele2->ResetStele();

    if (Stele3)
        Stele3->ResetStele();

    bIsSolved = false;

    if (bShowDebugMessages)
    {
        UE_LOG(LogTemp, Log, TEXT("Puzzle reset"));
    }
}

void AStelepuzzlemanager::GetCurrentSymbols(ESymbolType& Symbol1, ESymbolType& Symbol2, ESymbolType& Symbol3) const
{
    Symbol1 = Stele1 ? Stele1->GetCurrentFrontSymbol() : ESymbolType::Sphere;
    Symbol2 = Stele2 ? Stele2->GetCurrentFrontSymbol() : ESymbolType::Sphere;
    Symbol3 = Stele3 ? Stele3->GetCurrentFrontSymbol() : ESymbolType::Sphere;
}