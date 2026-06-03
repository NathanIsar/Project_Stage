#include "BaseInteractableActor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"

ABaseInteractableActor::ABaseInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(RootComponent);
}

void ABaseInteractableActor::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseInteractableActor::Interact_Implementation(AActor* Interactor)
{
	if (!Interactor) return;
	
	if (!Execute_CanInteract(this)) return;
	
	bIsEnabled = false;
	GetWorldTimerManager().SetTimer(
		CooldownTimerHandle,
		[this]() { bIsEnabled = true; },
		InteractionCooldown,
		false
	);

	OnInteracted(Interactor);

	if (bDebugMode)
		UE_LOG(LogTemp, Log, TEXT("%s interacted with by %s"), *GetName(), *Interactor->GetName());
}

bool ABaseInteractableActor::CanInteract_Implementation() const
{
	if (!bIsEnabled) return false;
	
	if (bRequiresLineOfSight)
	{
		UWorld* World = GetWorld();
		if (!World) return false;

		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC || !PC->GetPawn()) return false;

		FVector TraceStart = PC->GetPawn()->GetActorLocation();
		FVector TraceEnd = GetActorLocation();

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		Params.AddIgnoredActor(PC->GetPawn());

		if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
		{
			return false;
		}
	}

	return true;
}

void ABaseInteractableActor::OnInteractionRangeEntered_Implementation(AActor* Interactor)
{
	OnPlayerEnterRange(Interactor);
	
	if (bDebugMode)
		UE_LOG(LogTemp, Log, TEXT("%s: Player entered range"), *GetName());
}

void ABaseInteractableActor::OnInteractionRangeExited_Implementation(AActor* Interactor)
{
	OnPlayerExitRange(Interactor);

	if (bDebugMode)
		UE_LOG(LogTemp, Log, TEXT("%s: Player exited range"), *GetName());
}

FText ABaseInteractableActor::GetActionName_Implementation() const
{
	return FText::FromString("Interact");
}
