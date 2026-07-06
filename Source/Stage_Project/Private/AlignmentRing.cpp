#include "AlignmentRing.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

AAlignmentRing::AAlignmentRing()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    RingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
    RingMesh->SetupAttachment(Root);

    SymbolDisk = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SymbolDisk"));
    SymbolDisk->SetupAttachment(Root);

    SymbolTarget = CreateDefaultSubobject<USceneComponent>(TEXT("SymbolTarget"));
    SymbolTarget->SetupAttachment(SymbolDisk);
}

void AAlignmentRing::BeginPlay()
{
    Super::BeginPlay();

    if (SymbolDisk)
    {
        if (SymbolMaterial)
        {
            SymbolDisk->SetMaterial(0, SymbolMaterial);
        }
        DiskMID = SymbolDisk->CreateAndSetMaterialInstanceDynamic(0);
    }
}

void AAlignmentRing::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bFading)
    {
        FadeElapsed += DeltaSeconds;
        const float Alpha = FMath::Clamp(FadeElapsed / FadeDuration, 0.f, 1.f);
        if (DiskMID)
        {
            DiskMID->SetScalarParameterValue(DissolveParamName, Alpha);
        }
        if (Alpha >= 1.f)
        {
            bFading = false;
            if (SymbolDisk) { SymbolDisk->SetVisibility(false); }
        }
        return;
    }

    if (bSolved) { return; }

    if (IsPlayerAligned())
    {
        AlignHold += DeltaSeconds;
        if (AlignHold >= HoldTime)
        {
            bSolved = true;
            StartFade();
            OnRingAligned.Broadcast(this);
        }
    }
    else
    {
        AlignHold = 0.f;
    }
}

bool AAlignmentRing::IsPlayerAligned() const
{
    const APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!Cam || !SymbolTarget) { return false; }

    const FVector C = Cam->GetCameraLocation();
    const FVector D = Cam->GetActorForwardVector().GetSafeNormal();
    const FVector R = RingMesh ? RingMesh->GetComponentLocation() : GetActorLocation();
    const FVector S = SymbolTarget->GetComponentLocation();
    
    auto DistPointToRay = [&C, &D](const FVector& P, float& OutT) -> float
    {
        OutT = FVector::DotProduct(P - C, D);
        return FVector::Dist(C + D * OutT, P);
    };
    
    float tR;
    if (DistPointToRay(R, tR) > HoleRadius || tR <= 0.f) { return false; }
    
    float tS;
    if (DistPointToRay(S, tS) > SymbolRadius || tS <= tR) { return false; }
    
    if (MaxViewDistance > 0.f && FVector::Dist(C, R) > MaxViewDistance) { return false; }
    
    if (bRequireLineOfSight)
    {
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        if (const APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
        {
            Params.AddIgnoredActor(P);
        }
        if (GetWorld()->LineTraceSingleByChannel(Hit, C, S, ECC_Visibility, Params))
        {
            return false;
        }
    }

    return true;
}

void AAlignmentRing::StartFade()
{
    bFading = true;
    FadeElapsed = 0.f;

    if (SolveSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SolveSound, GetActorLocation(), SolveVolume);
    }
}