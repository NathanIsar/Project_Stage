#include "GrabComponent.h"
#include "IGrabbable.h"
#include "PuzzleStele.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"   

UGrabComponent::UGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGrabComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* Owner = Cast<ACharacter>(GetOwner()))
	{
		PlayerCamera = Owner->FindComponentByClass<UCameraComponent>();
		if (!PlayerCamera) return;
	}

	// ─── Hover timer ──────────────────────────────────────────────────────────
	GetWorld()->GetTimerManager().SetTimer(
		HoverTimerHandle,
		this,
		&UGrabComponent::HoverTimerTick,
		HoverCheckInterval,
		true
	);

	// ─── Spawn de l'actor de prévisualisation ──────────────────────────────────
	if (PreviewActorClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		PreviewActor = GetWorld()->SpawnActor<ATrajectoryPreviewActor>(
			PreviewActorClass,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			Params
		);
	}
}

// ─── Clic Gauche ─────────────────────────────────────────────────────────────

void UGrabComponent::TryGrabOrRelease()
{
	if (HeldActor)
	{
		Release();
		return;
	}

	if (AActor* Target = LineTraceForGrabbable())
		Grab(Target);
}

// ─── Clic Droit Pressed → Aim ────────────────────────────────────────────────

void UGrabComponent::StartThrowAim()
{
	if (!HeldActor) return;

	bIsThrowReady = true;
	OnThrowReadyChanged.Broadcast(true);

	UpdateTrajectory();
	GetWorld()->GetTimerManager().SetTimer(
		TrajectoryTimerHandle,
		this,
		&UGrabComponent::UpdateTrajectory,
		TrajectoryRefreshRate,
		true
	);
}

// ─── Clic Droit Released → Throw ─────────────────────────────────────────────

void UGrabComponent::ReleaseThrow()
{
	if (!bIsThrowReady || !HeldActor) return;

	HideTrajectory();
	ExecuteThrow();
}

// ─── Annulation aim sans lancer ──────────────────────────────────────────────

void UGrabComponent::CancelThrowAim()
{
	if (!bIsThrowReady) return;

	HideTrajectory();
	bIsThrowReady = false;
	OnThrowReadyChanged.Broadcast(false);
}

// ─── Grab ─────────────────────────────────────────────────────────────────────

void UGrabComponent::Grab(AActor* Target)
{
	if (!Target || !PlayerCamera) return;

	if (ANumberGrab* Digit = Cast<ANumberGrab>(Target))
	{
		if (Digit->bIsPlaced)
		{
			TArray<AActor*> Steles;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleStele::StaticClass(), Steles);
			for (AActor* A : Steles)
			{
				APuzzleStele* Stele = Cast<APuzzleStele>(A);
				for (UPuzzleSlotComponent* Slot : Stele->Slots)
				{
					if (Slot && Slot->PlacedDigit == Digit)
					{
						Slot->EjectDigit();
						break;
					}
				}
			}
		}
	}

	if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		bWasSimulatingPhysics = Prim->IsSimulatingPhysics();
		Prim->SetSimulatePhysics(false);
		Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SavedPawnResponse = Prim->GetCollisionResponseToChannel(ECC_Pawn);
		Prim->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	FAttachmentTransformRules Rules(
		EAttachmentRule::KeepWorld,
		EAttachmentRule::KeepWorld,
		EAttachmentRule::KeepWorld,
		false
	);

	if (HoldSocketName != NAME_None)
		Target->AttachToComponent(PlayerCamera, Rules, HoldSocketName);
	else
		Target->AttachToComponent(PlayerCamera, Rules);

	Target->SetActorRelativeLocation(FVector(HoldDistance, 0.f, 0.f));
	Target->SetActorRelativeRotation(FRotator::ZeroRotator);

	HeldActor = Target;

	if (Target->Implements<UIGrabbable>())
		IIGrabbable::Execute_OnGrabbed(Target, GetOwner());

	OnActorGrabbed.Broadcast(Target);

	if (GrabSound)
	{
		if (SlotAudioComponent && SlotAudioComponent->IsPlaying())
			SlotAudioComponent->Stop();
		SlotAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			this, GrabSound, Target->GetActorLocation());
	}

	GetWorld()->GetTimerManager().PauseTimer(HoverTimerHandle);
}

// ─── Release (Clic Gauche) ───────────────────────────────────────────────────

void UGrabComponent::Release()
{
	if (!HeldActor) return;

	if (bIsThrowReady)
	{
		HideTrajectory();
		bIsThrowReady = false;
		OnThrowReadyChanged.Broadcast(false);
	}

	AActor* Released      = HeldActor;
	const FVector DropLoc = Released->GetActorLocation();

	Released->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Released->GetRootComponent()))
	{
		Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Prim->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnResponse);
		Prim->SetSimulatePhysics(true);
	}

	if (Released->Implements<UIGrabbable>())
		IIGrabbable::Execute_OnReleased(Released, DropLoc);

	OnActorReleased.Broadcast(Released);

	if (ReleaseSound)
	{
		if (SlotAudioComponent && SlotAudioComponent->IsPlaying())
			SlotAudioComponent->Stop();
		SlotAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			this, ReleaseSound, Released->GetActorLocation());
	}

	HeldActor = nullptr;

	if (ANumberGrab* Digit = Cast<ANumberGrab>(Released))
	{
		TArray<AActor*> Steles;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APuzzleStele::StaticClass(), Steles);
		for (AActor* A : Steles)
		{
			if (Cast<APuzzleStele>(A)->TrySnapDigitToNearestSlot(Digit))
				break;
		}
	}

	GetWorld()->GetTimerManager().UnPauseTimer(HoverTimerHandle);
}

// ─── ExecuteThrow ─────────────────────────────────────────────────────────────

void UGrabComponent::ExecuteThrow()
{
	if (!HeldActor || !PlayerCamera) return;

	AActor* Target         = HeldActor;
	const FVector Velocity = PlayerCamera->GetForwardVector() * ThrowForce;
	const FVector DropLoc  = Target->GetActorLocation();

	Target->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Target->GetRootComponent()))
	{
		Prim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Prim->SetCollisionResponseToChannel(ECC_Pawn, SavedPawnResponse);
		Prim->SetSimulatePhysics(true);
		Prim->SetPhysicsLinearVelocity(Velocity);
	}

	if (Target->Implements<UIGrabbable>())
		IIGrabbable::Execute_OnReleased(Target, DropLoc);

	OnActorReleased.Broadcast(Target);
	OnActorThrown.Broadcast(Target);

	if (ThrowSound)
	{
		if (SlotAudioComponent && SlotAudioComponent->IsPlaying())
			SlotAudioComponent->Stop();
		SlotAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			this, ThrowSound, DropLoc);
	}

	HeldActor     = nullptr;
	bIsThrowReady = false;
	OnThrowReadyChanged.Broadcast(false);

	GetWorld()->GetTimerManager().UnPauseTimer(HoverTimerHandle);
}

// ─── Trajectoire ─────────────────────────────────────────────────────────────

void UGrabComponent::UpdateTrajectory()
{
	if (!HeldActor || !PlayerCamera) return;

	FPredictProjectilePathParams Params;
	Params.StartLocation      = HeldActor->GetActorLocation();
	Params.LaunchVelocity     = PlayerCamera->GetForwardVector() * ThrowForce;
	Params.bTraceWithCollision = true;
	Params.ProjectileRadius   = TrajectoryProjectileRadius;
	Params.MaxSimTime         = TrajectoryMaxTime;
	Params.bTraceWithChannel  = true;
	Params.TraceChannel       = ECC_Visibility;
	Params.SimFrequency       = TrajectorySimFrequency;
	Params.ActorsToIgnore.Add(GetOwner());
	Params.ActorsToIgnore.Add(HeldActor);

	FPredictProjectilePathResult Result;
	UGameplayStatics::PredictProjectilePath(GetWorld(), Params, Result);

	ThrowTrajectoryPoints.Reset(Result.PathData.Num());
	for (const FPredictProjectilePathPointData& Pt : Result.PathData)
		ThrowTrajectoryPoints.Add(Pt.Location);

	if (Result.HitResult.IsValidBlockingHit())
		ThrowLandingPoint = Result.HitResult.ImpactPoint;
	else if (ThrowTrajectoryPoints.Num() > 0)
		ThrowLandingPoint = ThrowTrajectoryPoints.Last();
	
	if (PreviewActor)
		PreviewActor->UpdateTrajectory(ThrowTrajectoryPoints, ThrowLandingPoint);
	
	OnTrajectoryUpdated.Broadcast(ThrowTrajectoryPoints);
}

void UGrabComponent::HideTrajectory()
{
	GetWorld()->GetTimerManager().ClearTimer(TrajectoryTimerHandle);
	ThrowTrajectoryPoints.Empty();
	ThrowLandingPoint = FVector::ZeroVector;

	if (PreviewActor) PreviewActor->Hide();

	OnTrajectoryHidden.Broadcast();
}

// ─── Hover ───────────────────────────────────────────────────────────────────

void UGrabComponent::HoverTimerTick()
{
	UpdateLookAt();
}

void UGrabComponent::UpdateLookAt()
{
	if (HeldActor) { SetHoveredActor(nullptr); return; }
	SetHoveredActor(LineTraceForGrabbable());
}

AActor* UGrabComponent::LineTraceForGrabbable() const
{
	if (!PlayerCamera) return nullptr;

	const FVector Start = PlayerCamera->GetComponentLocation();
	const FVector End   = Start + PlayerCamera->GetForwardVector() * ReachDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UIGrabbable>())
			return HitActor;
	}

	return nullptr;
}

void UGrabComponent::SetHoveredActor(AActor* NewHovered)
{
	if (NewHovered == HoveredActor) return;

	if (HoveredActor && HoveredActor->Implements<UIGrabbable>())
		IIGrabbable::Execute_OnUnhovered(HoveredActor);

	AActor* Old  = HoveredActor;
	HoveredActor = NewHovered;

	if (HoveredActor && HoveredActor->Implements<UIGrabbable>())
		IIGrabbable::Execute_OnHovered(HoveredActor);

	OnHoverChanged.Broadcast(Old, HoveredActor);
}