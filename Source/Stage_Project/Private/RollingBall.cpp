// Fill out your copyright notice in the Description page of Project Settings.


#include "RollingBall.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

ARollingBall::ARollingBall()
{
	PrimaryActorTick.bCanEverTick = false;

	SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	RootComponent = SphereMesh;

	SphereMesh->SetSimulatePhysics(true);
	SphereMesh->SetNotifyRigidBodyCollision(true);
	SphereMesh->SetGenerateOverlapEvents(true); 

	SphereMesh->OnComponentHit.AddDynamic(this, &ARollingBall::OnSphereHit);
}

void ARollingBall::OnSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character) return;
	
	FVector PushDir = Character->GetActorLocation() - GetActorLocation();
	PushDir.Z = 0.f;
	PushDir.Normalize();

	const FVector LaunchVelocity = PushDir * PushForce + FVector(0.f, 0.f, PushUpForce);
	
	Character->LaunchCharacter(LaunchVelocity, true, true);
}

