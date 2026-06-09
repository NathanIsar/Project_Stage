#include "InteractionComponent.h"
#include "IInteractable.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetSphereRadius(InteractionRange);
	
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionObjectType(ECC_WorldDynamic); // La sphère elle-même est vue comme dynamique
	
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	
	InteractionSphere->SetGenerateOverlapEvents(true);
}

void UInteractionComponent::OnRegister()
{
	Super::OnRegister();
	if (InteractionSphere && GetOwner())
	{
		InteractionSphere->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (InteractionSphere)
	{
		InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &UInteractionComponent::OnOverlapBegin);
		InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &UInteractionComponent::OnOverlapEnd);
	}
	
	if (AActor* MyOwner = GetOwner())
	{
		PlayerCamera = MyOwner->FindComponentByClass<UCameraComponent>();
	}
	
	GetWorld()->GetTimerManager().SetTimer(HUDUpdateTimerHandle, this, &UInteractionComponent::UpdateHUDVisibility, 0.1f, true);
}

void UInteractionComponent::UpdateHUDVisibility()
{
	if (!PlayerCamera) return;

	AActor* NewTarget = nullptr;
	FVector Start = PlayerCamera->GetComponentLocation();
	FVector End   = Start + (PlayerCamera->GetForwardVector() * HUDRange); 

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();

		if (bDebugMode && HitActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("HUD Trace touche : %s | Interface : %d | InRange : %d"),
				*HitActor->GetName(),
				(int32)HitActor->Implements<UIInteractable>(),
				(int32)InteractablesInRange.Contains(HitActor));
		}

		if (HitActor && HitActor->Implements<UIInteractable>() && IIInteractable::Execute_CanInteract(HitActor))
			NewTarget = HitActor;
	}

	if (bDebugMode)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Cyan,
			FString::Printf(TEXT("Objets à portée : %d"), InteractablesInRange.Num()));
	}
	
	if (NewTarget != FocusedInteractable)
	{
		AActor* OldFocused   = FocusedInteractable;
		FocusedInteractable  = NewTarget;

		if (OldFocused)        OnInteractableLost.Broadcast();
		if (FocusedInteractable) OnInteractableDetected.Broadcast(FocusedInteractable);
	}
}

void UInteractionComponent::TryInteract()
{
	if (!bCanInteract)
	{
		if (bDebugMode)
			UE_LOG(LogTemp, Warning, TEXT("Interaction on cooldown"));
		return;
	}
	
	if (!CurrentInteractable)
	{
		AActor* Owner = GetOwner();
		if (!Owner) return;

		FVector Start = Owner->GetActorLocation();
		FVector End = Start + Owner->GetActorForwardVector() * InteractionRange;

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			if (Hit.GetActor() && Hit.GetActor()->Implements<UIInteractable>())
			{
				CurrentInteractable = Hit.GetActor();
			}
		}

		if (!CurrentInteractable)
		{
			if (bDebugMode)
				UE_LOG(LogTemp, Warning, TEXT("No interactable found"));
			return;
		}
	}
	
	if (!IIInteractable::Execute_CanInteract(CurrentInteractable))
	{
		if (bDebugMode)
			UE_LOG(LogTemp, Warning, TEXT("Cannot interact with %s"), *CurrentInteractable->GetName());
		return;
	}
	
	IIInteractable::Execute_Interact(CurrentInteractable, GetOwner());

	FString InteractedActorName = CurrentInteractable->GetName();
	
	AActor* NewBest = FindBestInteractable();
	if (NewBest != CurrentInteractable)
	{
		UpdateCurrentInteractable(NewBest);
	}
	
	bCanInteract = false;
	GetWorld()->GetTimerManager().SetTimer(
		CooldownTimerHandle,
		this,
		&UInteractionComponent::ResetCooldown,
		GlobalCooldown,
		false
	);

	if (bDebugMode)
		UE_LOG(LogTemp, Log, TEXT("Interacted with %s"), *InteractedActorName);
}

void UInteractionComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetOwner() && OtherActor->Implements<UIInteractable>())
	{
		IIInteractable::Execute_OnInteractionRangeEntered(OtherActor, GetOwner());
		
		InteractablesInRange.AddUnique(OtherActor);
		
		if (!CurrentInteractable)
		{
			UpdateCurrentInteractable(OtherActor);
		}
		else
		{
			AActor* BestInteractable = FindBestInteractable();
			if (BestInteractable != CurrentInteractable)
			{
				UpdateCurrentInteractable(BestInteractable);
			}
		}

		if (bDebugMode)
			UE_LOG(LogTemp, Log, TEXT("Interactable entered range: %s"), *OtherActor->GetName());
	}
}

void UInteractionComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor->Implements<UIInteractable>())
	{
		IIInteractable::Execute_OnInteractionRangeExited(OtherActor, GetOwner());
		
		InteractablesInRange.Remove(OtherActor);
		
		if (OtherActor == CurrentInteractable)
		{
			AActor* NewBest = FindBestInteractable();
			UpdateCurrentInteractable(NewBest);
		}

		if (bDebugMode)
			UE_LOG(LogTemp, Log, TEXT("Interactable left range: %s"), *OtherActor->GetName());
	}
}

AActor* UInteractionComponent::FindBestInteractable()
{
	if (!InteractionSphere) return nullptr;

	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	TArray<AActor*> OverlappingActors;
	InteractionSphere->GetOverlappingActors(OverlappingActors);

	AActor* BestActor = nullptr;
	float BestScore = -1.f;

	FVector OwnerLocation = Owner->GetActorLocation();
	FVector OwnerForward = Owner->GetActorForwardVector();

	for (AActor* Actor : OverlappingActors)
	{
		if (!Actor || Actor == Owner || !Actor->Implements<UIInteractable>())
			continue;

		if (!IIInteractable::Execute_CanInteract(Actor))
			continue;
		
		FVector ToActor = (Actor->GetActorLocation() - OwnerLocation).GetSafeNormal();
		float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
		float DotProduct = FVector::DotProduct(OwnerForward, ToActor);
		
		float Score = DotProduct * (1.f - (Distance / InteractionRange));

		if (Score > BestScore)
		{
			BestScore = Score;
			BestActor = Actor;
		}
	}

	return BestActor;
}

void UInteractionComponent::ResetCooldown()
{
	bCanInteract = true;
}

void UInteractionComponent::UpdateCurrentInteractable(AActor* NewInteractable)
{
	if (CurrentInteractable != NewInteractable)
	{
		CurrentInteractable = NewInteractable;
	}
}