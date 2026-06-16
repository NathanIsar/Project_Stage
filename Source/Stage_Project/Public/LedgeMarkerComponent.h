// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LedgeMarkerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STAGE_PROJECT_API ULedgeMarkerComponent : public USceneComponent
{
	GENERATED_BODY()
 
public:
	ULedgeMarkerComponent();
 
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge")
	float LedgeLength = 200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge")
	bool bCanMantle = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge")
	bool bShowDebug = true;
	
	FVector GetLedgeDirection() const { return GetRightVector(); }
	
	FVector GetLedgeNormal() const { return GetForwardVector(); }
 
	FVector GetLedgeStart() const { return GetComponentLocation() - GetRightVector() * (LedgeLength * 0.5f); }
	FVector GetLedgeEnd()   const { return GetComponentLocation() + GetRightVector() * (LedgeLength * 0.5f); }
	
	FVector GetClosestPoint(const FVector& WorldPos) const;
	
	static const TArray<TWeakObjectPtr<ULedgeMarkerComponent>>& GetAll() { return Registry; }
 
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
 
private:
	static TArray<TWeakObjectPtr<ULedgeMarkerComponent>> Registry;
};
