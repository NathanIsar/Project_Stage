#include "PuzzleSlotComponent.h"
#include "Components/PrimitiveComponent.h"


UPuzzleSlotComponent::UPuzzleSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	SetCollisionProfileName(TEXT("OverlapAll"));
	SetGenerateOverlapEvents(true);
	
	SetHiddenInGame(false);
}
 
bool UPuzzleSlotComponent::TryPlaceDigit(ANumberGrab* Digit)
{
	if (!Digit || !IsEmpty()) return false;
	
	if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Digit->GetRootComponent()))
	{
		Prim->SetSimulatePhysics(false);
		Prim->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	
	FAttachmentTransformRules Rules(
	   EAttachmentRule::SnapToTarget,  
	   EAttachmentRule::SnapToTarget,  
	   EAttachmentRule::KeepWorld,    
	   true
   );
	
	Digit->AttachToComponent(this, Rules);

	FBoxSphereBounds SlotBounds = CalcBounds(GetComponentTransform());
	float SlotHalfHeight = SlotBounds.BoxExtent.Z;

	FVector DigitOrigin, DigitExtent;
	Digit->GetActorBounds(false, DigitOrigin, DigitExtent);
	
	FVector TopOfSlot = GetComponentLocation();
	TopOfSlot.Z += SlotHalfHeight + DigitExtent.Z;

	Digit->SetActorLocation(TopOfSlot);
	Digit->SetActorRotation(FRotator::ZeroRotator);
	Digit->bIsPlaced = true;
	PlacedDigit = Digit;
	
	if (Digit->GrabbedMaterial)
		Digit->MeshComponent->SetMaterial(0, Digit->GrabbedMaterial);
 
	OnSlotChanged.Broadcast(this);
	return true;
}
 
void UPuzzleSlotComponent::EjectDigit()
{
	if (!PlacedDigit) return;
 
	PlacedDigit->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
 
	if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(PlacedDigit->GetRootComponent()))
	{
		Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Prim->SetSimulatePhysics(true);
	}
 
	PlacedDigit->bIsPlaced = false;
	PlacedDigit = nullptr;
 
	OnSlotChanged.Broadcast(this);
}
 
int32 UPuzzleSlotComponent::GetPlacedValue() const
{
	return PlacedDigit ? PlacedDigit->DigitValue : -1;
}

