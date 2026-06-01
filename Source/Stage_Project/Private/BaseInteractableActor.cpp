#include "BaseInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ABaseInteractableActor::ABaseInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create mesh component for visual feedback
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
}

void ABaseInteractableActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseInteractableActor::Interact_Implementation(AActor* Interactor)
{
	if (!CanInteract_Implementation())
		return;

	
	OnInteracted(Interactor);

	UE_LOG(LogTemp, Log, TEXT("%s interacted with by %s"), *GetName(), *Interactor->GetName());
}

bool ABaseInteractableActor::CanInteract_Implementation() const
{
	return bIsEnabled;
}

void ABaseInteractableActor::OnInteractionRangeEntered_Implementation(AActor* Interactor)
{
	OnPlayerEnterRange(Interactor);
	UE_LOG(LogTemp, Log, TEXT("%s: Player entered range"), *GetName());
}

void ABaseInteractableActor::OnInteractionRangeExited_Implementation(AActor* Interactor)
{
	OnPlayerExitRange(Interactor);
	UE_LOG(LogTemp, Log, TEXT("%s: Player exited range"), *GetName());
}


