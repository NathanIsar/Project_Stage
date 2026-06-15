#include "BaseGrabbableActor.h"
#include "Components/StaticMeshComponent.h"


ABaseGrabbableActor::ABaseGrabbableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;
	MeshComponent->SetSimulatePhysics(false);
	MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));
}

void ABaseGrabbableActor::BeginPlay()
{
	Super::BeginPlay();

	// Auto-capture le matériau du mesh si rien n'est assigné dans l'éditeur
	if (!DefaultMaterial && MeshComponent && MeshComponent->GetMaterial(0))
	{
		DefaultMaterial = MeshComponent->GetMaterial(0);
	}
} 

void ABaseGrabbableActor::OnGrabbed_Implementation(AActor* Grabber)
{
	bIsGrabbed     = true;
	CurrentGrabber = Grabber;

	ApplyMaterial(GrabbedMaterial);
	OnGrabbedEvent(Grabber);
}

void ABaseGrabbableActor::OnReleased_Implementation(FVector DropLocation)
{
	bIsGrabbed       = false;
	CurrentGrabber   = nullptr;
	LastDropLocation = DropLocation;

	ApplyMaterial(DefaultMaterial);
	OnReleasedEvent(DropLocation);
}

void ABaseGrabbableActor::OnHovered_Implementation()
{
	bIsHovered = true;
	OnHoveredEvent();
}

void ABaseGrabbableActor::OnUnhovered_Implementation()
{
	bIsHovered = false;
	if (!bIsGrabbed)
	OnUnhoveredEvent();
}

FText ABaseGrabbableActor::GetActionName_Implementation() const
{
	return InteractLabel;
}

void ABaseGrabbableActor::ApplyMaterial(UMaterialInterface* Material)
{
	if (Material && MeshComponent)
		MeshComponent->SetMaterial(0, Material);
}