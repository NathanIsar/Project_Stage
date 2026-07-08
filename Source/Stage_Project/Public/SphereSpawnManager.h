#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SphereSpawnManager.generated.h"

UCLASS()
class STAGE_PROJECT_API ASphereSpawnManager : public AActor
{
	GENERATED_BODY()

public:
	ASphereSpawnManager();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> SphereClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<TObjectPtr<AActor>> SpawnPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.1"))
	float SpawnInterval = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 ActiveSpawnIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bAutoStart = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "0.0"))
	float SphereLifetime = 12.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float InitialForwardImpulse = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void StopSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawn")
	void SetActiveSpawnIndex(int32 NewIndex);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<int32> MaxSpawnsPerLane;
	
	TArray<int32> SpawnCountPerLane;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void SpawnSphere();

	FTimerHandle SpawnTimerHandle;
};