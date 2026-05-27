// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Sound/SoundBase.h"
#include "PuzzleDoor.generated.h"

class UAudioComponent;
UCLASS()
class STAGE_PROJECT_API APuzzleDoor : public AActor
{
	GENERATED_BODY()
	
public:
	APuzzleDoor();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Components")
	UStaticMeshComponent* DoorMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UBoxComponent* BlockingCollision;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Config")
	float DitherDuration = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Config")
	UCurveFloat* DitherCurve;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Config")
	FName DitherParamName = FName("Dither_Opacity");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|FX")
	USoundBase* OpenSound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|FX")
    TSubclassOf<UCameraShakeBase> OpenCameraShakeClass;
	
	UFUNCTION(BlueprintCallable, Category="Door")
	void Open();
 
protected:
	virtual void BeginPlay() override;
 
private:
	FTimerHandle  DitherTimerHandle;
	float         ElapsedTime = 0.f;
	bool          bIsOpening  = false;
	
	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;
	
	static constexpr float TimerInterval = 0.016f;
	
	UPROPERTY()
	UAudioComponent* OpenAudioComponent = nullptr;
	
	UFUNCTION()
	void UpdateDither();
 
	void FinishOpen();
};
