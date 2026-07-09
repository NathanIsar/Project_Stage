// Copyright STAGE_PROJECT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldWrapper.generated.h"

class UBoxComponent;

UCLASS()
class STAGE_PROJECT_API AWorldWrapper : public AActor
{
	GENERATED_BODY()

public:
	AWorldWrapper();
	
	UFUNCTION(BlueprintCallable, Category = "World Wrapper")
	void ReleaseActor(AActor* Target);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Wrapper")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Wrapper")
	TObjectPtr<UBoxComponent> TopBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "World Wrapper")
	TObjectPtr<UBoxComponent> BottomBox;


	UPROPERTY(EditAnywhere, Category = "World Wrapper", meta = (ClampMin = "0.0"))
	float MaxWrapSpeed = 3000.f;
	
	UPROPERTY(EditAnywhere, Category = "World Wrapper",
		meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "!bZeroGravityInTube"))
	float SpeedCorrectionAlpha = 0.35f;
	
	UPROPERTY(EditAnywhere, Category = "World Wrapper")
	bool bZeroGravityInTube = true;
	
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "World Wrapper", meta = (ClampMin = "0.0"))
	float WorstCaseDeltaTime = 0.1f;

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void Wrap(AActor* Target, const UBoxComponent* From, const UBoxComponent* To);
	void WrapCharacter(class ACharacter* Char, const FVector& Dest);
	void WrapPhysicsActor(AActor* Target, const FVector& Dest) const;
	
	bool IsRelevantComponent(const AActor* OtherActor, const UPrimitiveComponent* OtherComp) const;

	float ApplySpeedLimit(float VerticalSpeed) const;
	void ValidateBoxThickness(const UBoxComponent* Box) const;
	
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<const UBoxComponent>> WrapGuard;
	
	TMap<TWeakObjectPtr<AActor>, float> SavedGravityScales;
};