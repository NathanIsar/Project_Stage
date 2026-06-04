#include "SteleActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ASteleActor::ASteleActor()
{
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base"));
    BaseMesh->SetupAttachment(RootComponent);
    
    SteleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stele"));
    SteleMesh->SetupAttachment(BaseMesh);
}

void ASteleActor::BeginPlay()
{
    Super::BeginPlay();
    
    StartRotation = SteleMesh->GetRelativeRotation();
    
    CurrentRotationIndex = 0;
}

void ASteleActor::Interact_Implementation(AActor* Interactor)
{
    if (!CanInteract_Implementation())
        return;
    
    if (bIsRotating)
    {
        UE_LOG(LogTemp, Warning, TEXT("Stele is already rotating!"));
        return;
    }
    
    bool bShouldRotateClockwise = bClockwiseRotation;
    if (bRotateBasedOnPlayerSide && Interactor)
    {
        bShouldRotateClockwise = ShouldRotateClockwise(Interactor);
    }
    
    StartRotation2(bShouldRotateClockwise);
    
    Super::Interact_Implementation(Interactor);
}

bool ASteleActor::CanInteract_Implementation() const
{
    return Super::CanInteract_Implementation() && !bIsRotating;
}

void ASteleActor::StartRotation2(bool bClockwise)
{
    bIsRotating = true;
    OnRotationStarted();
    
    if (RotationSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, RotationSound, GetActorLocation());
    }
    
    ESymbolType OldSymbol = GetCurrentFrontSymbol();
    
    if (bClockwise)
    {
        CurrentRotationIndex = (CurrentRotationIndex + 1) % 4;
    }
    else
    {
        CurrentRotationIndex = (CurrentRotationIndex - 1 + 4) % 4;
    }
    
    StartRotation = SteleMesh->GetRelativeRotation();
    float RotationAngle = bClockwise ? 90.f : -90.f;
    TargetRotation = StartRotation + FRotator(0.f, RotationAngle, 0.f);
    
    float TimerInterval = 0.016f;
    GetWorld()->GetTimerManager().SetTimer(
        RotationTimerHandle,
        this,
        &ASteleActor::UpdateRotation,
        TimerInterval,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Stele rotation started - Old: %d, New: %d"),
        static_cast<int32>(OldSymbol),
        static_cast<int32>(GetCurrentFrontSymbol()));
}

void ASteleActor::UpdateRotation()
{
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    RotationElapsedTime += DeltaTime;
    
    float Alpha = FMath::Clamp(RotationElapsedTime / RotationDuration, 0.f, 1.f);
    Alpha = FMath::SmoothStep(0.f, 1.f, Alpha);

    FRotator NewRotation = FMath::Lerp(StartRotation, TargetRotation, Alpha);
    SteleMesh->SetRelativeRotation(NewRotation);

    if (Alpha >= 1.f)
    {
        RotationElapsedTime = 0.f;
        CompleteRotation();
    }
}

void ASteleActor::CompleteRotation()
{
    
    GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
    
    
    SteleMesh->SetRelativeRotation(TargetRotation);
    
    bIsRotating = false;
    
    ESymbolType NewSymbol = GetCurrentFrontSymbol();
    
    if (RotationCompleteSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, RotationCompleteSound, GetActorLocation());
    }
    
    OnRotationCompleted(NewSymbol);
    OnSteleRotated.Broadcast(NewSymbol);

    UE_LOG(LogTemp, Log, TEXT("Stele rotation completed - Current symbol: %d"), 
        static_cast<int32>(NewSymbol));
}

ESymbolType ASteleActor::GetCurrentFrontSymbol() const
{
    return GetSymbolAtIndex(CurrentRotationIndex);
}

ESymbolType ASteleActor::GetSymbolAtIndex(int32 Index) const
{
    switch (Index)
    {
    case 0: return FrontSymbol;
    case 1: return RightSymbol;
    case 2: return BackSymbol;
    case 3: return LeftSymbol;
    default: return FrontSymbol;
    }
}

void ASteleActor::ResetStele()
{
    if (bIsRotating)
    {
        GetWorld()->GetTimerManager().ClearTimer(RotationTimerHandle);
        bIsRotating = false;
    }

    CurrentRotationIndex = 0;
    SteleMesh->SetRelativeRotation(FRotator::ZeroRotator);

    UE_LOG(LogTemp, Log, TEXT("Stele reset"));
}

void ASteleActor::SetToSymbol(ESymbolType TargetSymbol)
{
    for (int32 i = 0; i < 4; i++)
    {
        if (GetSymbolAtIndex(i) == TargetSymbol)
        {
            CurrentRotationIndex = i;
            float YawRotation = i * 90.f;
            SteleMesh->SetRelativeRotation(FRotator(0.f, YawRotation, 0.f));
            
            OnSteleRotated.Broadcast(TargetSymbol);
            
            UE_LOG(LogTemp, Log, TEXT("Stele set to symbol: %d"), static_cast<int32>(TargetSymbol));
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Symbol not found on this stele!"));
}

bool ASteleActor::ShouldRotateClockwise(AActor* Interactor) const
{
    if (!Interactor)
        return bClockwiseRotation;
    
    FVector ToPlayer = Interactor->GetActorLocation() - GetActorLocation();
    ToPlayer.Normalize();
    

    FVector ActorRight = GetActorRightVector();
    
    float DotRight = FVector::DotProduct(ToPlayer, ActorRight);
    
    return DotRight > 0.f;
}