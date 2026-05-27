// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IGrabbable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UIGrabbable : public UInterface
{
	GENERATED_BODY()
};

class STAGE_PROJECT_API IIGrabbable
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGrabbed(AActor* Grabber);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnReleased(FVector DropLocation);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnHovered();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnUnhovered();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FText GetInteractLabel();
};
