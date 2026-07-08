#pragma once

#include "CoreMinimal.h"
#include "BaseInteractableActor.h"
#include "Capstan.generated.h"

class ASphereSpawnManager;

UCLASS()
class STAGE_PROJECT_API ACapstan : public ABaseInteractableActor
{
	GENERATED_BODY()

public:
	ACapstan();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capstan")
	TObjectPtr<ASphereSpawnManager> SpawnManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capstan")
	TArray<int32> LaneCycle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capstan")
	float DegreesPerTurn = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capstan")
	float RotationSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Capstan")
	bool bSingleUse = false;

	UFUNCTION(BlueprintCallable, Category = "Capstan")
	void TurnCapstan();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Interact_Implementation(AActor* Interactor) override;

	void StartRotation();
	void UpdateRotation();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capstan")
	class UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Capstan")
	class UStaticMeshComponent* WheelMesh;

	int32 CycleCursor = 0;
	float CurrentYaw = 0.0f;
	float TargetYaw = 0.0f;
	bool bLocked = false;

	FTimerHandle RotationTimerHandle;

	static constexpr float RotationTickInterval = 1.0f / 60.0f;
};