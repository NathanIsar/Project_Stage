// Copyright STAGE_PROJECT. All Rights Reserved.

#include "WorldWrapper.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogWorldWrapper, Log, All);

AWorldWrapper::AWorldWrapper()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	const FVector DefaultExtent(500.f, 500.f, 400.f);

	TopBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TopBox"));
	TopBox->SetupAttachment(SceneRoot);
	TopBox->SetBoxExtent(DefaultExtent);
	TopBox->SetCollisionProfileName(TEXT("Trigger"));

	BottomBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BottomBox"));
	BottomBox->SetupAttachment(SceneRoot);
	BottomBox->SetBoxExtent(DefaultExtent);
	BottomBox->SetCollisionProfileName(TEXT("Trigger"));
}

void AWorldWrapper::BeginPlay()
{
	Super::BeginPlay();

	TopBox->OnComponentBeginOverlap.AddDynamic(this, &AWorldWrapper::OnBoxBeginOverlap);
	BottomBox->OnComponentBeginOverlap.AddDynamic(this, &AWorldWrapper::OnBoxBeginOverlap);
	TopBox->OnComponentEndOverlap.AddDynamic(this, &AWorldWrapper::OnBoxEndOverlap);
	BottomBox->OnComponentEndOverlap.AddDynamic(this, &AWorldWrapper::OnBoxEndOverlap);

	ValidateBoxThickness(TopBox);
	ValidateBoxThickness(BottomBox);
}

void AWorldWrapper::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (const TPair<TWeakObjectPtr<AActor>, float>& Entry : SavedGravityScales)
	{
		if (ACharacter* Char = Cast<ACharacter>(Entry.Key.Get()))
		{
			if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
			{
				Move->GravityScale = Entry.Value;
			}
		}
	}
	SavedGravityScales.Empty();
	WrapGuard.Empty();

	Super::EndPlay(EndPlayReason);
}

void AWorldWrapper::ValidateBoxThickness(const UBoxComponent* Box) const
{
	if (!Box || MaxWrapSpeed <= 0.f)
	{
		return;
	}
	
	const float MinHalfThickness = MaxWrapSpeed * WorstCaseDeltaTime * 0.5f;
	const float ActualHalfThickness = Box->GetScaledBoxExtent().Z;

	if (ActualHalfThickness < MinHalfThickness)
	{
		UE_LOG(LogWorldWrapper, Warning,
			TEXT("[%s] %s trop fine : demi-epaisseur %.0f cm pour MaxWrapSpeed=%.0f cm/s. ")
			TEXT("Minimum conseille : %.0f cm. Risque de tunneling."),
			*GetName(), *Box->GetName(), ActualHalfThickness, MaxWrapSpeed, MinHalfThickness);
	}
}

bool AWorldWrapper::IsRelevantComponent(const AActor* OtherActor, const UPrimitiveComponent* OtherComp) const
{
	if (!OtherActor || !OtherComp || OtherActor == this)
	{
		return false;
	}
	
	if (const ACharacter* Char = Cast<ACharacter>(OtherActor))
	{
		return OtherComp == Char->GetCapsuleComponent();
	}

	return OtherComp == OtherActor->GetRootComponent();
}

void AWorldWrapper::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool /*bFromSweep*/, const FHitResult&SweepResult)
{
	if (!IsRelevantComponent(OtherActor, OtherComp))
	{
		return;
	}
	
	if (WrapGuard.Contains(OtherActor))
	{
		return;
	}

	const UBoxComponent* From = Cast<UBoxComponent>(OverlappedComp);
	const UBoxComponent* To = (From == BottomBox) ? TopBox.Get() : BottomBox.Get();

	if (!From || !To)
	{
		return;
	}

	WrapGuard.Add(OtherActor, To);
	Wrap(OtherActor, From, To);
}

void AWorldWrapper::OnBoxEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsRelevantComponent(OtherActor, OtherComp))
	{
		return;
	}
	
	if (const TWeakObjectPtr<const UBoxComponent>* Dest = WrapGuard.Find(OtherActor))
	{
		if (Dest->Get() == OverlappedComp)
		{
			WrapGuard.Remove(OtherActor);
		}
	}
}

void AWorldWrapper::Wrap(AActor* Target, const UBoxComponent* From, const UBoxComponent* To)
{
	const FVector Local = From->GetComponentTransform().InverseTransformPosition(Target->GetActorLocation());
	const FVector Dest = To->GetComponentTransform().TransformPosition(Local);

	if (ACharacter* Char = Cast<ACharacter>(Target))
	{
		WrapCharacter(Char, Dest);
		return;
	}

	WrapPhysicsActor(Target, Dest);
}

float AWorldWrapper::ApplySpeedLimit(float VerticalSpeed) const
{
	if (MaxWrapSpeed <= 0.f || VerticalSpeed >= -MaxWrapSpeed)
	{
		return VerticalSpeed;
	}
	
	return FMath::Lerp(VerticalSpeed, -MaxWrapSpeed, SpeedCorrectionAlpha);
}

void AWorldWrapper::WrapCharacter(ACharacter* Char, const FVector& Dest)
{
	UCharacterMovementComponent* Move = Char->GetCharacterMovement();
	if (!Move)
	{
		return;
	}

	FVector SavedVelocity = Move->Velocity;

	if (bZeroGravityInTube)
	{
		if (!SavedGravityScales.Contains(Char))
		{
			SavedGravityScales.Add(Char, Move->GravityScale);
			SavedVelocity.Z = FMath::Max(SavedVelocity.Z, -MaxWrapSpeed);
			Move->GravityScale = 0.f;
		}
	}
	else
	{
		SavedVelocity.Z = ApplySpeedLimit(SavedVelocity.Z);
	}
	
	Char->TeleportTo(Dest, Char->GetActorRotation(), /*bIsATest*/ false, /*bNoCheck*/ true);

	Move->Velocity = SavedVelocity;
	Move->SetMovementMode(MOVE_Falling);
	
	if (const APlayerController* PC = Cast<APlayerController>(Char->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetGameCameraCutThisFrame();
		}
	}
}

void AWorldWrapper::WrapPhysicsActor(AActor* Target, const FVector& Dest) const
{
	UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(Target->GetRootComponent());
	if (!Prim || !Prim->IsSimulatingPhysics())
	{
		Target->SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics);
		return;
	}

	FVector LinearVelocity = Prim->GetPhysicsLinearVelocity();
	const FVector AngularVelocity = Prim->GetPhysicsAngularVelocityInDegrees();

	LinearVelocity.Z = ApplySpeedLimit(LinearVelocity.Z);

	Target->SetActorLocation(Dest, false, nullptr, ETeleportType::TeleportPhysics);

	Prim->SetPhysicsLinearVelocity(LinearVelocity);
	Prim->SetPhysicsAngularVelocityInDegrees(AngularVelocity);
}

void AWorldWrapper::ReleaseActor(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	float OriginalScale = 1.f;
	if (SavedGravityScales.RemoveAndCopyValue(Target, OriginalScale))
	{
		if (ACharacter* Char = Cast<ACharacter>(Target))
		{
			if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
			{
				Move->GravityScale = OriginalScale;
			}
		}
	}

	WrapGuard.Remove(Target);
}