#include "DoorActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADoorActor::ADoorActor()
{
    DoorFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrame"));
    DoorFrame->SetupAttachment(RootComponent);

    DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
    DoorMesh->SetupAttachment(DoorFrame);
    
    PlayerTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("PlayerTrigger"));
    PlayerTrigger->SetupAttachment(RootComponent);
    PlayerTrigger->SetBoxExtent(FVector(50.f, 100.f, 100.f));
    PlayerTrigger->SetCollisionProfileName(TEXT("Trigger"));
    PlayerTrigger->SetGenerateOverlapEvents(false);
}

void ADoorActor::BeginPlay()
{
    Super::BeginPlay();

    ClosedRotation = DoorMesh->GetRelativeRotation();
    OpenRotation = ClosedRotation + FRotator(0.f, OpenAngle, 0.f);
    
    PlayerTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnPlayerEnterDoor);
}

void ADoorActor::UnlockAndOpen()
{
    UE_LOG(LogTemp, Warning, TEXT("=== UnlockAndOpen START ==="));
    UE_LOG(LogTemp, Log, TEXT("Current DoorState: %d"), static_cast<int32>(DoorState));
    
    if (DoorState == EDoorState::Open || DoorState == EDoorState::Opening)
    {
        UE_LOG(LogTemp, Warning, TEXT("Door is already open or opening!"));
        return;
    }
    
    GetWorld()->GetTimerManager().ClearTimer(LerpTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(AnimationTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
    

    
    DoorState = EDoorState::Closed;
    DoorMesh->SetRelativeRotation(ClosedRotation);
    
    UE_LOG(LogTemp, Log, TEXT("Opening door..."));
    
    OpenDoor();
    
    UE_LOG(LogTemp, Warning, TEXT("=== UnlockAndOpen END ==="));
}

void ADoorActor::OpenDoor()
{
    DoorState = EDoorState::Opening;
    
    if (OpenSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, OpenSound, GetActorLocation());
        UE_LOG(LogTemp, Log, TEXT("Playing open sound"));
    }
    
    LerpAlpha = 0.f;
    StartRotation = DoorMesh->GetRelativeRotation();
    TargetRotation = OpenRotation;
    
    GetWorld()->GetTimerManager().SetTimer(
        LerpTimerHandle,
        this,
        &ADoorActor::UpdateOpenAnimation,
        0.016f, 
        true     
    );

    UE_LOG(LogTemp, Log, TEXT("Door opening animation started..."));
}

void ADoorActor::CloseDoor()
{
    DoorState = EDoorState::Closing;
    
    GetWorld()->GetTimerManager().ClearTimer(AutoCloseTimerHandle);
    
    if (CloseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, CloseSound, GetActorLocation());
        UE_LOG(LogTemp, Log, TEXT("Playing close sound"));
    }
    
    LerpAlpha = 0.f;
    StartRotation = DoorMesh->GetRelativeRotation();
    TargetRotation = ClosedRotation;
    
    GetWorld()->GetTimerManager().SetTimer(
        LerpTimerHandle,
        this,
        &ADoorActor::UpdateCloseAnimation,
        0.016f,  
        true     
    );

    UE_LOG(LogTemp, Log, TEXT("Door closing animation started..."));
}

void ADoorActor::UpdateOpenAnimation()
{
    LerpAlpha += GetWorld()->GetDeltaSeconds() / OpenDuration;
    
    if (LerpAlpha >= 1.f)
    {
        LerpAlpha = 1.f;
        GetWorld()->GetTimerManager().ClearTimer(LerpTimerHandle);
        
 
        DoorMesh->SetRelativeRotation(TargetRotation);
        
        DoorState = EDoorState::Open;
        OnDoorOpened();
        
        PlayerTrigger->SetGenerateOverlapEvents(true);
        
        UE_LOG(LogTemp, Log, TEXT("Door fully opened!"));
    }
    else
    {
        float SmoothAlpha = FMath::SmoothStep(0.f, 1.f, LerpAlpha);
        FRotator NewRotation = FMath::Lerp(StartRotation, TargetRotation, SmoothAlpha);
        DoorMesh->SetRelativeRotation(NewRotation);
    }
}

void ADoorActor::UpdateCloseAnimation()
{
    LerpAlpha += GetWorld()->GetDeltaSeconds() / OpenDuration;
    
    if (LerpAlpha >= 1.f)
    {
        LerpAlpha = 1.f;
        GetWorld()->GetTimerManager().ClearTimer(LerpTimerHandle);
        
        DoorMesh->SetRelativeRotation(TargetRotation);
        
        DoorState = EDoorState::Closed;
        OnDoorClosed();
        
        PlayerTrigger->SetGenerateOverlapEvents(false);
        
        OnDoorManuallyClosed.Broadcast();
        
        UE_LOG(LogTemp, Log, TEXT("Door fully closed!"));
    }
    else
    {
        float SmoothAlpha = FMath::SmoothStep(0.f, 1.f, LerpAlpha);
        FRotator NewRotation = FMath::Lerp(StartRotation, TargetRotation, SmoothAlpha);
        DoorMesh->SetRelativeRotation(NewRotation);
    }
}

void ADoorActor::ScheduleAutoClose()
{
    GetWorld()->GetTimerManager().SetTimer(
        AutoCloseTimerHandle,
        this,
        &ADoorActor::CloseDoor,
        AutoCloseDelay,
        false
    );
}

void ADoorActor::OnPlayerEnterDoor(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA(APawn::StaticClass()))
        return;

    if (DoorState != EDoorState::Open)
        return;

    UE_LOG(LogTemp, Log, TEXT("Joueur a traversé la porte — fermeture."));
    CloseDoor();
}