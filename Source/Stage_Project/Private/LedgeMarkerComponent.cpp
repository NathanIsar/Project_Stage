#include "LedgeMarkerComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

TArray<TWeakObjectPtr<ULedgeMarkerComponent>> ULedgeMarkerComponent::Registry;

ULedgeMarkerComponent::ULedgeMarkerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bTickInEditor = true;
}

void ULedgeMarkerComponent::BeginPlay()
{
	Super::BeginPlay();
	Registry.AddUnique(this);
}

void ULedgeMarkerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Registry.Remove(this);
	Super::EndPlay(EndPlayReason);
}

void ULedgeMarkerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
										  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShowDebug && GetWorld())
	{
		const FVector A = GetLedgeStart();
		const FVector B = GetLedgeEnd();
		const FVector Mid = GetComponentLocation();
		DrawDebugLine(GetWorld(), A, B, FColor::Green, false, -1.f, 0, 3.f);
		DrawDebugPoint(GetWorld(), A, 8.f, FColor::Green, false, -1.f);
		DrawDebugPoint(GetWorld(), B, 8.f, FColor::Green, false, -1.f);
		DrawDebugLine(GetWorld(), Mid, Mid + GetLedgeNormal() * 40.f, FColor::Cyan, false, -1.f, 0, 2.f);
		DrawDebugPoint(GetWorld(), Mid, 10.f, bCanMantle ? FColor::Red : FColor::Yellow, false, -1.f);
	}
}

FVector ULedgeMarkerComponent::GetClosestPoint(const FVector& WorldPos) const
{
	const FVector A = GetLedgeStart();
	const FVector B = GetLedgeEnd();
	const FVector AB = B - A;
	const float L2 = AB.SizeSquared();
	if (L2 <= KINDA_SMALL_NUMBER) return A;

	float T = FVector::DotProduct(WorldPos - A, AB) / L2;
	T = FMath::Clamp(T, 0.f, 1.f);
	return A + AB * T;
}