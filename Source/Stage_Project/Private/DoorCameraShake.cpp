// Fill out your copyright notice in the Description page of Project Settings.


#include "DoorCameraShake.h"
#include "Shakes/PerlinNoiseCameraShakePattern.h"

UDoorCameraShake::UDoorCameraShake(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bSingleInstance = true;

	UPerlinNoiseCameraShakePattern* Pattern =
		CreateDefaultSubobject<UPerlinNoiseCameraShakePattern>(TEXT("DoorShakePattern"));

	Pattern->Duration     = 3.0f;
	Pattern->BlendInTime  = 0.05f;
	Pattern->BlendOutTime = 0.3f;

	Pattern->Pitch.Amplitude = 0.6f;
	Pattern->Pitch.Frequency = 12.f;
	Pattern->Roll.Amplitude  = 0.3f;
	Pattern->Roll.Frequency  = 10.f;
	Pattern->Z.Amplitude     = 1.5f;
	Pattern->Z.Frequency     = 15.f;

	SetRootShakePattern(Pattern);
}

