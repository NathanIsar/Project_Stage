// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"  
#include "GrabComponent.generated.h"



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STAGE_PROJECT_API UGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGrabComponent();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	float ReachDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	float HoldDistance = 150.f; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	FName HoldSocketName = NAME_None; 
	
	UPROPERTY(BlueprintReadOnly, Category="Grab|State")
	AActor* HeldActor = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	float HoverCheckInterval = 0.1f;

	UPROPERTY(BlueprintReadOnly, Category="Grab|State")
	AActor* HoveredActor = nullptr;
	
	UFUNCTION(BlueprintCallable, Category="Grab")
	void TryGrabOrRelease();

	UFUNCTION(BlueprintCallable, Category="Grab")
	void Release();
	
	UFUNCTION(BlueprintCallable, Category="Grab")
	void UpdateLookAt();
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabChanged, AActor*, Actor);

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnGrabChanged OnActorGrabbed;

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnGrabChanged OnActorReleased;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHoverChanged, AActor*, OldActor, AActor*, NewActor);

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnHoverChanged OnHoverChanged;

protected:
	
	virtual void BeginPlay() override;

public:	
	UPROPERTY()
	UCameraComponent* PlayerCamera = nullptr;

	bool bWasSimulatingPhysics = false;
	
	AActor* LineTraceForGrabbable() const;
	void    Grab(AActor* Target);
	void    SetHoveredActor(AActor* NewHovered);
	ECollisionResponse SavedPawnResponse = ECR_Block;
private:
	FTimerHandle HoverTimerHandle;
    
	UFUNCTION()
	void HoverTimerTick();
};
