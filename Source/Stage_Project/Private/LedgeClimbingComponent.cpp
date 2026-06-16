#include "LedgeClimbingComponent.h"
#include "LedgeMarkerComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ULedgeClimbingComponent::ULedgeClimbingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULedgeClimbingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULedgeClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDetectionCooldown)
	{
		DetectionCooldownTimer -= DeltaTime;
		if (DetectionCooldownTimer <= 0.f)
			bDetectionCooldown = false;
	}
	
	if (bAutoGrabbing)
	{
		AutoGrabTimer -= DeltaTime;
		if (AutoGrabTimer <= 0.f)
		{
			bAutoGrabbing = false;
		}
		else if (CurrentState == ELedgeState::None && !bDetectionCooldown)
		{
			ACharacter* Char = GetOwnerCharacter();
			if (Char)
			{
				FVector Pt;
				ULedgeMarkerComponent* M = FindNearestLedge(
					Char->GetActorLocation(), GrabRange, /*VertDir=*/+1, JumpFromLedgeZ, nullptr, Pt);
				if (M)
				{
					AttachToLedge(M, Pt);
					bAutoGrabbing = false;
				}
			}
		}
	}

	switch (CurrentState)
	{
	case ELedgeState::Hanging:  TickHanging(DeltaTime);  break;
	case ELedgeState::Climbing: TickClimbing(DeltaTime); break;
	case ELedgeState::Vaulting: TickVaulting(DeltaTime); break;
	default: break;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Recherche de rebord (marqueurs)
// ─────────────────────────────────────────────────────────────────────────────

ULedgeMarkerComponent* ULedgeClimbingComponent::FindNearestLedge(const FVector& From, float Range,
	int32 VertDir, float RefZ, ULedgeMarkerComponent* Skip, FVector& OutPoint) const
{
	ULedgeMarkerComponent* Best = nullptr;
	float BestDist2 = Range * Range;

	for (const TWeakObjectPtr<ULedgeMarkerComponent>& Weak : ULedgeMarkerComponent::GetAll())
	{
		ULedgeMarkerComponent* M = Weak.Get();
		if (!M || M == Skip) continue;

		const FVector P = M->GetClosestPoint(From);
		
		if (VertDir > 0 && P.Z <= RefZ + VerticalSeparation) continue;
		if (VertDir < 0 && P.Z >= RefZ - VerticalSeparation) continue;

		const float D2 = FVector::DistSquared(P, From);
		if (D2 < BestDist2)
		{
			BestDist2 = D2;
			Best      = M;
			OutPoint  = P;
		}
	}

	return Best;
}

void ULedgeClimbingComponent::AttachToLedge(ULedgeMarkerComponent* Marker, const FVector& Point)
{
	if (!Marker) return;

	CurrentLedge      = Marker;
	CurrentLedgePoint = Point;

	CurrentLedgeData.bIsValid         = true;
	CurrentLedgeData.LedgeTopPosition = Point;
	CurrentLedgeData.LedgeNormal      = Marker->GetLedgeNormal();
	CurrentLedgeData.bCanMantle       = Marker->bCanMantle;
	CurrentLedgeData.HangPosition     = ComputeHangPosition(CurrentLedgeData);

	GrabTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	SetState(ELedgeState::Hanging);
	PlayLedgeMontage(GrabMontage);
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void ULedgeClimbingComponent::TryGrabLedge()
{
	if (bDetectionCooldown) return;
	if (CurrentState != ELedgeState::None && CurrentState != ELedgeState::Detected) return;

	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	FVector Pt;
	ULedgeMarkerComponent* M = FindNearestLedge(Char->GetActorLocation(), GrabRange, 0, 0.f, nullptr, Pt);
	if (!M) return;

	AttachToLedge(M, Pt);
}

void ULedgeClimbingComponent::TryClimbUp()
{
	if (CurrentState != ELedgeState::Hanging) return;

	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	if (GetWorld() && (GetWorld()->GetTimeSeconds() - GrabTimeSeconds) < MantleInputLockoutTime)
		return;

	if (!CurrentLedgeData.bCanMantle)
		return;

	const float CapsuleHalfHeight = Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius     = Char->GetCapsuleComponent()->GetScaledCapsuleRadius();

	VaultStartLocation = Char->GetActorLocation();
	VaultEndLocation   = CurrentLedgePoint
		- CurrentLedgeData.LedgeNormal * (CapsuleRadius + 10.f)
		+ FVector(0.f, 0.f, CapsuleHalfHeight + 5.f);
	VaultTimer = 0.f;

	PlayLedgeMontage(VaultMontage ? VaultMontage : ClimbMontage);

	if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
	{
		Char->DisableInput(PC);
		bInputDisabled = true;
	}

	SetState(ELedgeState::Vaulting);
}

void ULedgeClimbingComponent::ReleaseLedge()
{
	if (CurrentState == ELedgeState::None) return;

	PlayLedgeMontage(ReleaseToGroundMontage);

	LateralDirection = 0.f;
	CurrentLedge.Reset();
	RestoreMovement();
	SetState(ELedgeState::None);
	CurrentLedgeData = FLedgeData();

	bDetectionCooldown     = true;
	DetectionCooldownTimer = DetectionCooldownDuration;
}

void ULedgeClimbingComponent::PlayLedgeMontage(UAnimMontage* Montage)
{
	if (!Montage) return;
	ACharacter* Char = GetOwnerCharacter();
	if (!Char || !Char->GetMesh()) return;
	if (UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance())
		Anim->Montage_Play(Montage);
}

void ULedgeClimbingComponent::StartLateralMove(float Direction)
{
	if (CurrentState != ELedgeState::Hanging && CurrentState != ELedgeState::Climbing) return;

	LateralDirection = FMath::Sign(Direction);
	if (!FMath::IsNearlyZero(LateralDirection))
		SetState(ELedgeState::Climbing);
}

void ULedgeClimbingComponent::StopLateralMove()
{
	LateralDirection = 0.f;
	if (CurrentState == ELedgeState::Climbing)
		SetState(ELedgeState::Hanging);
}

void ULedgeClimbingComponent::UpdateLateralInput(float RightAxis)
{
	if (CurrentState != ELedgeState::Hanging && CurrentState != ELedgeState::Climbing) return;

	const float DeadZone = 0.2f;
	if (FMath::Abs(RightAxis) > DeadZone)
		StartLateralMove(RightAxis);
	else
		StopLateralMove();
}

void ULedgeClimbingComponent::LedgeJump()
{
	if (CurrentState != ELedgeState::Hanging && CurrentState != ELedgeState::Climbing) return;

	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	const FVector LaunchVel =
		FVector::UpVector * LedgeJumpUpVelocity
		+ CurrentLedgeData.LedgeNormal * LedgeJumpOutVelocity;

	JumpFromLedgeZ = CurrentLedgePoint.Z;

	LateralDirection = 0.f;
	CurrentLedge.Reset();
	RestoreMovement();
	SetState(ELedgeState::None);
	CurrentLedgeData       = FLedgeData();
	bDetectionCooldown     = true;
	DetectionCooldownTimer = 0.15f;

	bAutoGrabbing = true;
	AutoGrabTimer = AutoGrabWindow;

	Char->LaunchCharacter(LaunchVel, true, true);
}

void ULedgeClimbingComponent::DropToLowerLedge()
{
	if (CurrentState != ELedgeState::Hanging && CurrentState != ELedgeState::Climbing) return;

	if (GetWorld() && (GetWorld()->GetTimeSeconds() - GrabTimeSeconds) < MantleInputLockoutTime)
		return;

	FVector Pt;
	ULedgeMarkerComponent* M = FindNearestLedge(
		CurrentLedgePoint, MaxDropDistance, -1, CurrentLedgePoint.Z, CurrentLedge.Get(), Pt);

	if (M)
	{
		LateralDirection = 0.f;
		AttachToLedge(M, Pt);
		PlayLedgeMontage(DropMontage);
	}
	else
	{
		ReleaseLedge();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick states
// ─────────────────────────────────────────────────────────────────────────────

void ULedgeClimbingComponent::TickHanging(float DeltaTime)
{
	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	const FVector TargetPos  = ComputeHangPosition(CurrentLedgeData);
	const FVector CurrentPos = Char->GetActorLocation();
	const FVector NewPos     = FMath::VInterpTo(CurrentPos, TargetPos, DeltaTime, HangSnapSpeed);
	Char->SetActorLocation(NewPos, false);
}

void ULedgeClimbingComponent::TickClimbing(float DeltaTime)
{
	if (FMath::IsNearlyZero(LateralDirection) || !CurrentLedge.IsValid())
	{
		SetState(ELedgeState::Hanging);
		return;
	}

	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;
	
	ULedgeMarkerComponent* M = CurrentLedge.Get();
	const FVector Dir     = M->GetLedgeDirection();
	const FVector Desired = CurrentLedgePoint + Dir * (LateralDirection * LateralMoveSpeed * DeltaTime);
	const FVector Clamped = M->GetClosestPoint(Desired);

	if (bDebugLedge && GEngine)
		GEngine->AddOnScreenDebugMessage(120, 0.5f, FColor::White,
			FString::Printf(TEXT("[Lateral] dir=%.0f"), LateralDirection));
	
	if (FVector::DistSquared(Clamped, CurrentLedgePoint) < 0.0625f)
	{
		LateralDirection = 0.f;
		SetState(ELedgeState::Hanging);
		return;
	}

	CurrentLedgePoint                 = Clamped;
	CurrentLedgeData.LedgeTopPosition = Clamped;

	const FVector TargetPos  = ComputeHangPosition(CurrentLedgeData);
	const FVector CurrentPos = Char->GetActorLocation();
	const FVector NewPos     = FMath::VInterpTo(CurrentPos, TargetPos, DeltaTime, HangSnapSpeed);
	Char->SetActorLocation(NewPos, false);
}

void ULedgeClimbingComponent::TickVaulting(float DeltaTime)
{
	VaultTimer += DeltaTime;
	const float Alpha = FMath::Clamp(VaultTimer / VaultDuration, 0.f, 1.f);

	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	const float SmoothAlpha = FMath::SmoothStep(0.f, 1.f, Alpha);
	const float ArcZ        = FMath::Sin(Alpha * PI) * 30.f;

	FVector NewPos = FMath::Lerp(VaultStartLocation, VaultEndLocation, SmoothAlpha);
	NewPos.Z      += ArcZ;
	Char->SetActorLocation(NewPos, true);

	if (Alpha >= 1.f)
	{
		Char->SetActorLocation(VaultEndLocation);
		CurrentLedge.Reset();
		RestoreMovement();
		SetState(ELedgeState::None);
		CurrentLedgeData       = FLedgeData();
		bDetectionCooldown     = true;
		DetectionCooldownTimer = DetectionCooldownDuration;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// State machine
// ─────────────────────────────────────────────────────────────────────────────

void ULedgeClimbingComponent::SetState(ELedgeState NewState)
{
	if (CurrentState == NewState) return;

	const ELedgeState OldState = CurrentState;
	CurrentState = NewState;

	if (NewState == ELedgeState::Hanging)
	{
		if (OldState != ELedgeState::Climbing)
			ApplyHangingPhysics();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Physics helpers
// ─────────────────────────────────────────────────────────────────────────────

void ULedgeClimbingComponent::ApplyHangingPhysics()
{
	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	if (!CMC) return;

	SavedMaxFlySpeed  = CMC->MaxFlySpeed;
	SavedMaxWalkSpeed = CMC->MaxWalkSpeed;

	CMC->SetMovementMode(MOVE_Flying);
	CMC->StopMovementImmediately();
	CMC->GravityScale = 0.f;
	CMC->MaxFlySpeed  = 0.f;

	if (AController* Controller = Char->GetController())
	{
		const float    WallFacingYaw = (-CurrentLedgeData.LedgeNormal).Rotation().Yaw;
		const FRotator ControlRot    = Controller->GetControlRotation();
		Controller->SetControlRotation(FRotator(ControlRot.Pitch, WallFacingYaw, 0.f));
	}
}

void ULedgeClimbingComponent::RestoreMovement()
{
	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	UCharacterMovementComponent* CMC = Char->GetCharacterMovement();
	if (!CMC) return;

	CMC->GravityScale = 1.f;
	CMC->MaxFlySpeed  = SavedMaxFlySpeed;
	CMC->MaxWalkSpeed = SavedMaxWalkSpeed;
	CMC->SetMovementMode(MOVE_Falling);

	if (bInputDisabled)
	{
		if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
			Char->EnableInput(PC);
		bInputDisabled = false;
	}
}

FVector ULedgeClimbingComponent::ComputeHangPosition(const FLedgeData& LedgeData) const
{
	const ACharacter* Char = GetOwnerCharacter();
	if (!Char) return FVector::ZeroVector;

	const float CapsuleHalfHeight = Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius     = Char->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float ForwardOffset = CapsuleRadius + HangWallGap;

	return LedgeData.LedgeTopPosition
		+ LedgeData.LedgeNormal * ForwardOffset
		+ FVector(0.f, 0.f, -CapsuleHalfHeight - HangDropOffset);
}

ACharacter* ULedgeClimbingComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}