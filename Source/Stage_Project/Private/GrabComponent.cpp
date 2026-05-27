
#include "GrabComponent.h"
#include "IGrabbable.h"
#include "PuzzleStele.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

UGrabComponent::UGrabComponent()
{
    PrimaryComponentTick.bCanEverTick = false; 
}

void UGrabComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
    {
        PlayerCamera = Owner->FindComponentByClass<UCameraComponent>();

        if (!PlayerCamera) return;
        PlayerCamera->GetComponentLocation();
        PlayerCamera->GetForwardVector();
    }
    
    GetWorld()->GetTimerManager().SetTimer(
    HoverTimerHandle,
    this,
    &UGrabComponent::HoverTimerTick,
    HoverCheckInterval,
    true 
);
}

void UGrabComponent::TryGrabOrRelease()
{
    if (HeldActor)
    {
        Release();
        return;
    }

    if (AActor* Target = LineTraceForGrabbable())
        Grab(Target);
}

void UGrabComponent::HoverTimerTick()
{
    UpdateLookAt(); 
}

void UGrabComponent::Grab(AActor* Target)
{
    if (!Target || !PlayerCamera) return;
    
    if (ANumberGrab* Digit = Cast<ANumberGrab>(Target))
    {
        if (Digit->bIsPlaced)
        {
            TArray<AActor*> Steles;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleStele::StaticClass(), Steles);
            for (AActor* A : Steles)
            {
                APuzzleStele* Stele = Cast<APuzzleStele>(A);
                for (UPuzzleSlotComponent* Slot : Stele->Slots)
                {
                    if (Slot && Slot->PlacedDigit == Digit)
                    {
                        Slot->EjectDigit();
                        break;
                    }
                }
            }
        }
    }
    // ────────────────────────────────────────────────────────────
    
    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
    {
        bWasSimulatingPhysics = Prim->IsSimulatingPhysics();
        Prim->SetSimulatePhysics(false);
        Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        SavedPawnResponse = Prim->GetCollisionResponseToChannel(ECC_Pawn);
        Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
    }
    
    FAttachmentTransformRules Rules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false);
    if (HoldSocketName != NAME_None)
        Target->AttachToComponent(PlayerCamera, Rules, HoldSocketName);
    else
        Target->AttachToComponent(PlayerCamera, Rules);
    
    Target->SetActorRelativeLocation(FVector(HoldDistance, 0.f, 0.f));
    Target->SetActorRelativeRotation(FRotator::ZeroRotator);

    HeldActor = Target;
    
    if (Target->Implements<UIGrabbable>())
        IIGrabbable::Execute_OnGrabbed(Target, GetOwner());

    OnActorGrabbed.Broadcast(Target);
    
    //if (GrabSound)
        //UGameplayStatics::PlaySoundAtLocation(this, GrabSound, Target->GetActorLocation());
    
    if (GrabSound)
    {
        if (SlotAudioComponent && SlotAudioComponent->IsPlaying())
            SlotAudioComponent->Stop();
 
        SlotAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            this, GrabSound, Target->GetActorLocation());
    }
    
    GetWorld()->GetTimerManager().PauseTimer(HoverTimerHandle);
}


void UGrabComponent::Release()
{
    if (!HeldActor) return;

    AActor* Released = HeldActor;
    FVector DropLocation = Released->GetActorLocation();

    Released->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    
    if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Released->GetRootComponent()))
    {
        Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Prim->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnResponse);
        Prim->SetSimulatePhysics(true);
    }
    
    if (Released->Implements<UIGrabbable>())
        IIGrabbable::Execute_OnReleased(Released, DropLocation);

    OnActorReleased.Broadcast(Released);
    
    //if (ReleaseSound)
        //UGameplayStatics::PlaySoundAtLocation(this, ReleaseSound, Released->GetActorLocation());
    
    if (ReleaseSound)
    {
        if (SlotAudioComponent && SlotAudioComponent->IsPlaying())
            SlotAudioComponent->Stop();
 
        SlotAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            this, ReleaseSound, Released->GetActorLocation());
    }
    
    HeldActor = nullptr;
    if (ANumberGrab* Digit = Cast<ANumberGrab>(Released))
    {
        TArray<AActor*> Steles;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleStele::StaticClass(), Steles);
        for (AActor* A : Steles)
        {
            if (Cast<APuzzleStele>(A)->TrySnapDigitToNearestSlot(Digit))
                break;
        }
    }
    GetWorld()->GetTimerManager().UnPauseTimer(HoverTimerHandle);
}

void UGrabComponent::UpdateLookAt()
{
    if (HeldActor)
    {
        SetHoveredActor(nullptr);
        return;
    }

    AActor* Looked = LineTraceForGrabbable();
    SetHoveredActor(Looked);
}

AActor* UGrabComponent::LineTraceForGrabbable() const
{
    if (!PlayerCamera) return nullptr;

    FVector  Start = PlayerCamera->GetComponentLocation();
    FVector  End   = Start + PlayerCamera->GetForwardVector() * ReachDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor->Implements<UIGrabbable>())
            return HitActor;
    }

    return nullptr;
}

void UGrabComponent::SetHoveredActor(AActor* NewHovered)
{
    if (NewHovered == HoveredActor) return;
    
    if (HoveredActor && HoveredActor->Implements<UIGrabbable>())
        IIGrabbable::Execute_OnUnhovered(HoveredActor);

    AActor* Old = HoveredActor;
    HoveredActor = NewHovered;
    
    if (HoveredActor && HoveredActor->Implements<UIGrabbable>())
        IIGrabbable::Execute_OnHovered(HoveredActor);

    OnHoverChanged.Broadcast(Old, HoveredActor);
}



