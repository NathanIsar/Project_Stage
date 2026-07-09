#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "FractureFeedback.generated.h"

class UGeometryCollectionComponent;
class UCameraShakeBase;
struct FChaosBreakEvent;

UCLASS()
class STAGE_PROJECT_API AFractureFeedback : public AActor
{
	GENERATED_BODY()

public:
	AFractureFeedback();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fracture")
	AActor* TargetWall = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fracture|FX")
	USoundBase* BreakSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fracture|FX")
	TSubclassOf<UCameraShakeBase> BreakCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fracture|FX")
	float CameraShakeScale = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fracture|Config")
	float MinBreakMass = 1.0f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnWallBreak(const FChaosBreakEvent& BreakEvent);

private:
	bool bHasTriggered = false;
};