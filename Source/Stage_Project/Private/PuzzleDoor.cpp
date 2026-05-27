#include "PuzzleDoor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

APuzzleDoor::APuzzleDoor()
{
	PrimaryActorTick.bCanEverTick = false;
 
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;
	DoorMesh->SetCollisionProfileName(TEXT("BlockAll"));
 
	BlockingCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingCollision"));
	BlockingCollision->SetupAttachment(RootComponent);
	BlockingCollision->SetCollisionProfileName(TEXT("BlockAll"));
	BlockingCollision->SetBoxExtent(FVector(10.f, 50.f, 100.f));
}
 
void APuzzleDoor::BeginPlay()
{
	Super::BeginPlay();
	
	if (DoorMesh->GetMaterial(0))
	{
		DynamicMaterial = DoorMesh->CreateAndSetMaterialInstanceDynamic(0);
		DynamicMaterial->SetScalarParameterValue(DitherParamName, 1.f);
	}
}
 
void APuzzleDoor::Open()
{
    if (bIsOpening) return;
    bIsOpening  = true;
    ElapsedTime = 0.f;

    BlockingCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (OpenSound)
	{
		OpenAudioComponent = UGameplayStatics::SpawnSoundAttached(
			OpenSound, 
			DoorMesh, 
			NAME_None, 
			FVector::ZeroVector, 
			EAttachLocation::KeepRelativeOffset, 
			false 
		);
	}
 

	if (OpenCameraShakeClass)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			PC->ClientStartCameraShake(OpenCameraShakeClass, 1.0f);
		}
	}

    GetWorld()->GetTimerManager().SetTimer(
        DitherTimerHandle, this, &APuzzleDoor::UpdateDither, TimerInterval, true
    );
}
 
void APuzzleDoor::UpdateDither()
{
	ElapsedTime += TimerInterval;
	
	const float RawAlpha = FMath::Clamp(ElapsedTime / DitherDuration, 0.f, 1.f);
    
	const float CurvedAlpha = DitherCurve
		? DitherCurve->GetFloatValue(RawAlpha)
		: RawAlpha;
	
	const float DitherValue = FMath::Lerp(1.f, 0.f, CurvedAlpha);
 
	if (DynamicMaterial)
		DynamicMaterial->SetScalarParameterValue(DitherParamName, DitherValue);
	
	if (OpenAudioComponent && OpenAudioComponent->IsPlaying())
	{
		OpenAudioComponent->SetVolumeMultiplier(DitherValue);
	}
	
	if (ElapsedTime >= DitherDuration)
		FinishOpen();
}
 
void APuzzleDoor::FinishOpen()
{
    GetWorld()->GetTimerManager().ClearTimer(DitherTimerHandle);

	if (OpenAudioComponent)
	{
		OpenAudioComponent->SetVolumeMultiplier(0.f);
		
		OpenAudioComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		OpenAudioComponent->bAutoDestroy = true;
		
	}
	
    UE_LOG(LogTemp, Warning, TEXT("FinishOpen appelé | AudioComponent: %s"),
    OpenAudioComponent ? TEXT("OK") : TEXT("NULL"));

    Destroy();
}
 
