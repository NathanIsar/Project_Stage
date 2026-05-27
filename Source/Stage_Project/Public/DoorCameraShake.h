// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "DoorCameraShake.generated.h"

UCLASS()
class STAGE_PROJECT_API UDoorCameraShake : public UCameraShakeBase
{
	GENERATED_BODY()
	
public:
	UDoorCameraShake(const FObjectInitializer& ObjectInitializer);
	
};
