#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TrajectoryPreviewActor.generated.h"

UCLASS(Blueprintable)
class STAGE_PROJECT_API ATrajectoryPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ATrajectoryPreviewActor();

	// ─── Config ────────────────────────────────────────────────────────────────
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trajectory|Config")
	UStaticMesh* DotMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trajectory|Config")
	UMaterialInterface* DotMaterial = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trajectory|Config", meta=(ClampMin="0.001"))
	float DotScale = 0.08f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trajectory|Config")
	UStaticMesh* LandingMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trajectory|Config")
	UMaterialInterface* LandingMaterial = nullptr;
	
	void UpdateTrajectory(const TArray<FVector>& Points, const FVector& LandingPoint);
	
	void Hide();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UInstancedStaticMeshComponent* TrajectoryDots = nullptr;
	
	UPROPERTY()
	UStaticMeshComponent* LandingIndicator = nullptr;
};