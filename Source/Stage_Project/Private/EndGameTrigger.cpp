#include "EndGameTrigger.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AEndGameTrigger::AEndGameTrigger()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;

    TriggerBox->SetBoxExtent(FVector(150.f, 150.f, 150.f));
    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionObjectType(ECC_WorldStatic);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AEndGameTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AEndGameTrigger::HandleOverlap);
}

void AEndGameTrigger::HandleOverlap(UPrimitiveComponent*, AActor* OtherActor,
                                    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (bTriggered) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (OtherActor != PlayerPawn) return;

    bTriggered = true;
    OnEndReached.Broadcast();
}