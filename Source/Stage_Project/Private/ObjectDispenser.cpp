#include "ObjectDispenser.h"
#include "BaseGrabbableActor.h"
#include "Engine/World.h"

AObjectDispenser::AObjectDispenser()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AObjectDispenser::Interact_Implementation(AActor* Interactor)
{
	if (!ObjectToDispense) return;

	CleanupDestroyedObjects();
	
	if (MaxActiveObjects > 0 && ActiveObjects.Num() >= MaxActiveObjects) return;
	
	FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(SpawnOffset);
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABaseGrabbableActor* Spawned = GetWorld()->SpawnActor<ABaseGrabbableActor>(
		ObjectToDispense,
		SpawnLocation,
		SpawnRotation,
		Params
	);

	if (!Spawned) return;

	ActiveObjects.Add(Spawned);
	OnObjectDispensed(Spawned);
	
	Super::Interact_Implementation(Interactor);
}

FText AObjectDispenser::GetActionName_Implementation() const
{
	return FText::FromString("Prendre");
}

void AObjectDispenser::CleanupDestroyedObjects()
{
	ActiveObjects.RemoveAll([](ABaseGrabbableActor* Obj)
	{
		return !IsValid(Obj);
	});
}