#include "RotatingStatueActor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

ARotatingStatueActor::ARotatingStatueActor()
{
	SocleMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SocleMesh"));
	SocleMeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetupAttachment(RootComponent);
}

void ARotatingStatueActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateOrientationState();
}

void ARotatingStatueActor::Interact_Implementation(AActor* Interactor)
{
	if (!Execute_CanInteract(this) || bIsRotating) return;

	// Gère le cooldown + broadcast OnInteracted (Blueprint) comme sur tes autres interactables.
	Super::Interact_Implementation(Interactor);

	bIsRotating = true;
	RotationElapsedTime = 0.f;
	RotationStartYaw = MeshComponent->GetComponentRotation().Yaw;
	RotationTargetYaw = RotationStartYaw + RotationStepDegrees;

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ARotatingStatueActor::TickRotation,
		RotationTickInterval,
		true
	);
}

void ARotatingStatueActor::TickRotation()
{
	RotationElapsedTime += RotationTickInterval;
	const float Alpha = FMath::Clamp(RotationElapsedTime / RotationDuration, 0.f, 1.f);
	const float SmoothAlpha = FMath::SmoothStep(0.f, 1.f, Alpha);

	const float NewYaw = FMath::Lerp(RotationStartYaw, RotationTargetYaw, SmoothAlpha);
	FRotator NewRotation = MeshComponent->GetComponentRotation();
	NewRotation.Yaw = NewYaw;
	MeshComponent->SetWorldRotation(NewRotation);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(RotationTimerHandle);
		bIsRotating = false;
		UpdateOrientationState();
	}
}

void ARotatingStatueActor::UpdateOrientationState()
{
	const float CurrentYaw = MeshComponent->GetComponentRotation().Yaw;
	const float Delta = FMath::Abs(GetShortestYawDelta(CurrentYaw, TargetYawWorld));

	const bool bWasCorrect = bIsCorrectOrientation;
	bIsCorrectOrientation = Delta <= OrientationTolerance;

	MeshComponent->SetCustomDepthStencilValue(bIsCorrectOrientation ? CorrectStencilValue : IncorrectStencilValue);

	if (bWasCorrect != bIsCorrectOrientation)
	{
		OnOrientationChanged.Broadcast(bIsCorrectOrientation);
	}
}

float ARotatingStatueActor::GetShortestYawDelta(float From, float To) const
{
	float Delta = FMath::Fmod(To - From, 360.f);
	if (Delta > 180.f) Delta -= 360.f;
	if (Delta < -180.f) Delta += 360.f;
	return Delta;
}

FText ARotatingStatueActor::GetActionName_Implementation() const
{
	return FText::FromString("Tourner");
}