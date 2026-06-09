#include "NumberGrab.h"

ANumberGrab::ANumberGrab()
{
}

void ANumberGrab::OnGrabbed_Implementation(AActor* Grabber)
{
	Super::OnGrabbed_Implementation(Grabber);
	bIsPlaced = false;
	OnDigitGrabbed.Broadcast();
}

void ANumberGrab::OnReleased_Implementation(FVector DropLocation)
{
	Super::OnReleased_Implementation(DropLocation);
	OnDigitReleased.Broadcast();
}

void ANumberGrab::OnHovered_Implementation()
{
	Super::OnHovered_Implementation();
	OnDigitHovered.Broadcast();
}

void ANumberGrab::OnUnhovered_Implementation()
{
	Super::OnUnhovered_Implementation();
	OnDigitUnhovered.Broadcast();
}

FText ANumberGrab::GetActionName_Implementation() const
{
	return FText::FromString("Grab");
}