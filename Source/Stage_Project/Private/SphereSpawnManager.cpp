#include "SphereSpawnManager.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ASphereSpawnManager::ASphereSpawnManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ASphereSpawnManager::BeginPlay()
{
    Super::BeginPlay();
    
    SpawnCountPerLane.Init(0, SpawnPoints.Num());
    
    if (bAutoStart)
    {
        StartSpawning();
    }
}

void ASphereSpawnManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopSpawning();
    Super::EndPlay(EndPlayReason);
}

void ASphereSpawnManager::StartSpawning()
{
    if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(SpawnTimerHandle))
    {
        GetWorld()->GetTimerManager().SetTimer(
            SpawnTimerHandle, this, &ASphereSpawnManager::SpawnSphere,
            SpawnInterval, true, SpawnInterval);
    }
}

void ASphereSpawnManager::StopSpawning()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    }
}

void ASphereSpawnManager::SetActiveSpawnIndex(int32 NewIndex)
{
    if (SpawnPoints.IsValidIndex(NewIndex))
    {
        ActiveSpawnIndex = NewIndex;
    }
}

void ASphereSpawnManager::SpawnSphere()
{
    if (!SphereClass || !SpawnPoints.IsValidIndex(ActiveSpawnIndex))
        return;
    
    if (MaxSpawnsPerLane.IsValidIndex(ActiveSpawnIndex))
    {
        const int32 Limit = MaxSpawnsPerLane[ActiveSpawnIndex];
        if (Limit > 0) 
        {
            if (!SpawnCountPerLane.IsValidIndex(ActiveSpawnIndex))
            {
                SpawnCountPerLane.SetNumZeroed(SpawnPoints.Num());
            }
            if (SpawnCountPerLane[ActiveSpawnIndex] >= Limit)
            {
                return; 
            }
        }
    }

    AActor* SpawnPoint = SpawnPoints[ActiveSpawnIndex];
    if (!SpawnPoint)
        return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    Params.Owner = this;

    AActor* NewSphere = GetWorld()->SpawnActor<AActor>(
        SphereClass, SpawnPoint->GetActorTransform(), Params);
    if (!NewSphere)
        return;
    
    if (SpawnCountPerLane.IsValidIndex(ActiveSpawnIndex))
    {
        SpawnCountPerLane[ActiveSpawnIndex]++;
    }

    if (SphereLifetime > 0.0f)
    {
        NewSphere->SetLifeSpan(SphereLifetime);
    }

    if (InitialForwardImpulse != 0.0f)
    {
        if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(NewSphere->GetRootComponent()))
        {
            if (Prim->IsSimulatingPhysics())
            {
                Prim->AddImpulse(
                    SpawnPoint->GetActorForwardVector() * InitialForwardImpulse,
                    NAME_None, true);
            }
        }
    }
}