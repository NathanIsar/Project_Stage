#include "BaseGrabbableActor.h"
#include "Components/StaticMeshComponent.h"


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