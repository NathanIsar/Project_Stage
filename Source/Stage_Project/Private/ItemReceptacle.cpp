#include "ItemReceptacle.h"
#include "PuzzleDoor.h"
#include "Components/BoxComponent.h"

AItemReceptacle::AItemReceptacle()
{
    PrimaryActorTick.bCanEverTick = false;

    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    RootComponent = BaseMesh;

    DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
    DetectionBox->SetupAttachment(RootComponent);
    DetectionBox->SetBoxExtent(FVector(50.f, 50.f, 50.f));
    DetectionBox->SetCollisionProfileName(TEXT("OverlapAll"));

    bAlreadyTriggered = false;
}

void AItemReceptacle::BeginPlay()
{
    Super::BeginPlay();
    DetectionBox->OnComponentBeginOverlap.AddDynamic(this, &AItemReceptacle::OnOverlapBegin);
}

void AItemReceptacle::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (bAlreadyTriggered || !OtherActor || OtherActor == this) return;
    
    if (!RequiredItemClass || !OtherActor->IsA(RequiredItemClass)) return;

    UPrimitiveComponent* PhysComp = Cast<UPrimitiveComponent>(OtherActor->GetRootComponent());
    if (!PhysComp || !PhysComp->IsSimulatingPhysics()) return;

    bAlreadyTriggered = true;

    if (LinkedDoor)
    {
        LinkedDoor->Open();
    }
}