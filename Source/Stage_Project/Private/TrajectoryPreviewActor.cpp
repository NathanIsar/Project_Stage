#include "TrajectoryPreviewActor.h"

ATrajectoryPreviewActor::ATrajectoryPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	TrajectoryDots = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("TrajectoryDots"));
	TrajectoryDots->SetupAttachment(Root);
	TrajectoryDots->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrajectoryDots->SetCastShadow(false);

	LandingIndicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LandingIndicator"));
	LandingIndicator->SetupAttachment(Root);
	LandingIndicator->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LandingIndicator->SetCastShadow(false);
	LandingIndicator->SetVisibility(false);
}

void ATrajectoryPreviewActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (DotMesh)
	{
		TrajectoryDots->SetStaticMesh(DotMesh);
		if (DotMaterial) TrajectoryDots->SetMaterial(0, DotMaterial);
	}

	if (LandingMesh)
	{
		LandingIndicator->SetStaticMesh(LandingMesh);
		if (LandingMaterial) LandingIndicator->SetMaterial(0, LandingMaterial);
	}
	
	SetActorHiddenInGame(true);
}

void ATrajectoryPreviewActor::UpdateTrajectory(const TArray<FVector>& Points, const FVector& LandingPoint)
{
	if (Points.Num() < 2)
	{
		Hide();
		return;
	}

	SetActorHiddenInGame(false);
	TrajectoryDots->ClearInstances();

	for (const FVector& Pt : Points)
	{
		FTransform T;
		T.SetLocation(Pt);
		T.SetScale3D(FVector(DotScale));
		TrajectoryDots->AddInstance(T, /* bWorldSpace= */ true);
	}
	
	LandingIndicator->SetWorldLocation(LandingPoint);
	LandingIndicator->SetVisibility(true);
}

void ATrajectoryPreviewActor::Hide()
{
	SetActorHiddenInGame(true);
	TrajectoryDots->ClearInstances();
	LandingIndicator->SetVisibility(false);
}