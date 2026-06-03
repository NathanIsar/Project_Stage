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
	
	if (!DefaultMaterial && MeshComponent && MeshComponent->GetMaterial(0))
	{
		DefaultMaterial = MeshComponent->GetMaterial(0);
	}
}


void ABaseGrabbableActor::OnGrabbed_Implementation(AActor* Grabber)
{
	bIsGrabbed     = true;
	CurrentGrabber = Grabber;

	if (bDisableCollisionWhileGrabbed)
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (bSimulatePhysicsWhileGrabbed)
		MeshComponent->SetSimulatePhysics(true);

	ApplyMaterial(GrabbedMaterial);

	OnGrabbedEvent(Grabber);
}

void ABaseGrabbableActor::OnReleased_Implementation(FVector DropLocation)
{
	bIsGrabbed      = false;
	CurrentGrabber  = nullptr;
	LastDropLocation = DropLocation;

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetSimulatePhysics(false);
	
	ApplyMaterial(bIsHovered ? HoveredMaterial : DefaultMaterial);

	OnReleasedEvent(DropLocation);
	
}

void ABaseGrabbableActor::OnHovered_Implementation()
{
	bIsHovered = true;

	if (!bIsGrabbed)
		ApplyMaterial(HoveredMaterial);

	OnHoveredEvent();
	
}

void ABaseGrabbableActor::OnUnhovered_Implementation()
{
	bIsHovered = false;

	if (!bIsGrabbed)
		ApplyMaterial(DefaultMaterial);

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