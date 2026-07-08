#include "Capstan.h"
#include "SphereSpawnManager.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

ACapstan::ACapstan()
{
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
    BaseMesh->SetupAttachment(RootComponent);

    WheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel"));
    WheelMesh->SetupAttachment(BaseMesh);
}

void ACapstan::BeginPlay()
{
    Super::BeginPlay();

    if (LaneCycle.Num() == 0)
    {
        LaneCycle = {0, 1};
    }

    if (WheelMesh)
    {
        CurrentYaw = WheelMesh->GetRelativeRotation().Yaw;
        TargetYaw  = CurrentYaw;
    }
}

void ACapstan::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GetWorld())
    {
        GetWorldTimerManager().ClearTimer(RotationTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

void ACapstan::Interact_Implementation(AActor* Interactor)
{
    if (!Interactor || !Execute_CanInteract(this))
        return;

    Super::Interact_Implementation(Interactor);

    TurnCapstan();
}

void ACapstan::TurnCapstan()
{
    if (bLocked || LaneCycle.Num() == 0)
        return;

    CycleCursor = (CycleCursor + 1) % LaneCycle.Num();
    const int32 TargetLane = LaneCycle[CycleCursor];

    if (SpawnManager)
    {
        SpawnManager->SetActiveSpawnIndex(TargetLane);
    }

    TargetYaw += DegreesPerTurn;
    StartRotation();

    if (bSingleUse)
    {
        bLocked = true;
    }
}

void ACapstan::StartRotation()
{
    if (GetWorld() && !GetWorldTimerManager().IsTimerActive(RotationTimerHandle))
    {
        GetWorldTimerManager().SetTimer(
            RotationTimerHandle, this, &ACapstan::UpdateRotation,
            RotationTickInterval, /*bLoop=*/true);
    }
}

void ACapstan::UpdateRotation()
{
    if (!WheelMesh)
    {
        GetWorldTimerManager().ClearTimer(RotationTimerHandle);
        return;
    }

    CurrentYaw = FMath::FInterpConstantTo(
        CurrentYaw, TargetYaw, RotationTickInterval, RotationSpeed);

    FRotator R = WheelMesh->GetRelativeRotation();
    R.Yaw = CurrentYaw;
    WheelMesh->SetRelativeRotation(R);

    if (FMath::IsNearlyEqual(CurrentYaw, TargetYaw, 0.05f))
    {
        R.Yaw = TargetYaw;
        WheelMesh->SetRelativeRotation(R);
        CurrentYaw = TargetYaw;
        GetWorldTimerManager().ClearTimer(RotationTimerHandle);
    }
}