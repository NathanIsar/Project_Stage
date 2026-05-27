#include "NumberGrab.h"

ANumberGrab::ANumberGrab()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
}

void ANumberGrab::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANumberGrab::OnGrabbed_Implementation(AActor* Grabber)
{
	bIsGrabbed = true;
	bIsPlaced  = false;
    
	if (GrabbedMaterial)
		MeshComponent->SetMaterial(0, GrabbedMaterial);
    
	OnDigitGrabbed.Broadcast();
}

void ANumberGrab::OnReleased_Implementation(FVector DropLocation)
{
	bIsGrabbed = false;
    
	if (DefaultMaterial)
		MeshComponent->SetMaterial(0, DefaultMaterial);
    
	OnDigitReleased.Broadcast();
}

void ANumberGrab::OnHovered_Implementation()
{
	OnDigitHovered.Broadcast();
}

void ANumberGrab::OnUnhovered_Implementation()
{
	OnDigitUnhovered.Broadcast();
}

FText ANumberGrab::GetInteractLabel_Implementation()
{
	return FText::Format(
		FText::FromString(TEXT("Prendre le {0}")),
		FText::AsNumber(DigitValue)
	);
}



