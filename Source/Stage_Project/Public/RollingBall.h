// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RollingBall.generated.h"

UCLASS()
class STAGE_PROJECT_API ARollingBall : public AActor
{
	GENERATED_BODY()

public:
	ARollingBall();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere")
	TObjectPtr<UStaticMeshComponent> SphereMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere|Push")
	float PushForce = 900.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sphere|Push")
	float PushUpForce = 300.0f;

protected:
	UFUNCTION()
	void OnSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
