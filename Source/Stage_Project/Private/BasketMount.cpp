#include "BasketMount.h"
#include "Components/StaticMeshComponent.h"

ABasketMount::ABasketMount()
{
	PrimaryActorTick.bCanEverTick = true;

	BasketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BasketMesh"));
	SetRootComponent(BasketMesh);
}

void ABasketMount::BeginPlay()
{
	Super::BeginPlay();
	
	SetActorTickEnabled(false);
}

void ABasketMount::AddRotationDelta(const FRotator& Delta)
{
	if (bIsRotating)
	{
		if (bQueueRotationsWhileBusy)
		{
			PendingDeltas.Add(Delta);
		}
		return;
	}

	StartRotation = GetActorRotation();
	TargetRotation = StartRotation + Delta;
	ElapsedTime = 0.0f;
	bIsRotating = true;
	SetActorTickEnabled(true);
}

void ABasketMount::PropagateRotation(const FRotator& Delta, TArray<ABasketMount*>& VisitedBaskets)
{
	if (VisitedBaskets.Contains(this))
	{
		return;
	}
	VisitedBaskets.Add(this);

	AddRotationDelta(Delta);

	for (const FGearConnection& Connection : GearConnections)
	{
		if (!Connection.ConnectedBasket)
		{
			continue;
		}

		const float SignedRatio = Connection.bInvertDirection ? -Connection.GearRatio : Connection.GearRatio;
		const FRotator PropagatedDelta = Delta * SignedRatio;

		Connection.ConnectedBasket->PropagateRotation(PropagatedDelta, VisitedBaskets);
	}
}

void ABasketMount::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsRotating)
	{
		return;
	}

	ElapsedTime += DeltaTime;
	const float Alpha = FMath::Clamp(ElapsedTime / FMath::Max(RotationDuration, KINDA_SMALL_NUMBER), 0.0f, 1.0f);
	const FQuat StartQuat = StartRotation.Quaternion();
	const FQuat TargetQuat = TargetRotation.Quaternion();
	SetActorRotation(FQuat::Slerp(StartQuat, TargetQuat, Alpha));

	if (Alpha >= 1.0f)
	{
		bIsRotating = false;
		OnRotationComplete.Broadcast(this);

		if (PendingDeltas.Num() > 0)
		{
			const FRotator NextDelta = PendingDeltas[0];
			PendingDeltas.RemoveAt(0);
			AddRotationDelta(NextDelta);
		}
		else
		{
			SetActorTickEnabled(false);
		}
	}
}

bool ABasketMount::IsOpenFaceFacing(const FVector& WorldDirection, float ToleranceDegrees) const
{
	const FVector OpenFaceDirection = GetActorForwardVector();
	const float DotThreshold = FMath::Cos(FMath::DegreesToRadians(ToleranceDegrees));
	return FVector::DotProduct(OpenFaceDirection.GetSafeNormal(), WorldDirection.GetSafeNormal()) >= DotThreshold;
}