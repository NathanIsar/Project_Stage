#include "LedgeClimbingComponent.h"

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

	switch (CurrentState)
	{
	case ELedgeState::Hanging:  TickHanging(DeltaTime);  break;
	case ELedgeState::Climbing: TickClimbing(DeltaTime); break;
	case ELedgeState::Vaulting: TickVaulting(DeltaTime); break;
	default: break;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Public API
// ─────────────────────────────────────────────────────────────────────────────

void ULedgeClimbingComponent::TryGrabLedge()
{
	if (bDetectionCooldown) return;
	if (CurrentState != ELedgeState::None && CurrentState != ELedgeState::Detected) return;

	FLedgeData Detected;
	if (!DetectLedge(Detected)) return;

	CurrentLedgeData = Detected;
	GrabTimeSeconds  = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	SetState(ELedgeState::Hanging);
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
	VaultStartLocation = Char->GetActorLocation();
	VaultEndLocation   = CurrentLedgeData.LedgeTopPosition + FVector(0.f, 0.f, CapsuleHalfHeight + 2.f);
	VaultEndLocation  += -CurrentLedgeData.LedgeNormal * 40.f;
	VaultStartRotation = Char->GetActorRotation();
	VaultTimer         = 0.f;
	
	if (UAnimInstance* Anim = Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr)
	{
		UAnimMontage* Montage = VaultMontage ? VaultMontage : ClimbMontage;
		if (Montage) Anim->Montage_Play(Montage);
	}

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

	LateralDirection = 0.f;
	RestoreMovement();
	SetState(ELedgeState::None);
	CurrentLedgeData = FLedgeData();

	bDetectionCooldown     = true;
	DetectionCooldownTimer = DetectionCooldownDuration;
}

void ULedgeClimbingComponent::StartLateralMove(float Direction)
{
	if (CurrentState != ELedgeState::Hanging && CurrentState != ELedgeState::Climbing) return;

	LateralDirection = FMath::Sign(Direction);
	if (!FMath::IsNearlyZero(LateralDirection))
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(104, 1.f, FColor::Cyan,
				FString::Printf(TEXT("[Lateral] Climbing ON, dir = %.0f"), LateralDirection));
		SetState(ELedgeState::Climbing);
	}
}

void ULedgeClimbingComponent::StopLateralMove()
{
	LateralDirection = 0.f;
	if (CurrentState == ELedgeState::Climbing)
		SetState(ELedgeState::Hanging);
}

void ULedgeClimbingComponent::UpdateLateralInput(float RightAxis)
{
	if (bDebugLedge && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(101, 1.f, FColor::Yellow,
			FString::Printf(TEXT("[Lateral] X recu = %.3f | State = %d"),
				RightAxis, (int32)CurrentState));
	}

	if (CurrentState != ELedgeState::Hanging && CurrentState != ELedgeState::Climbing)
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(102, 1.f, FColor::Red,
				TEXT("[Lateral] IGNORE : pas en Hanging/Climbing"));
		return;
	}

	const float DeadZone = 0.2f;
	if (FMath::Abs(RightAxis) > DeadZone)
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(103, 1.f, FColor::Green,
				FString::Printf(TEXT("[Lateral] -> StartLateralMove(%.2f)"), RightAxis));
		StartLateralMove(RightAxis);
	}
	else
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(103, 1.f, FColor::Orange,
				FString::Printf(TEXT("[Lateral] X sous la dead zone (%.3f <= 0.2) -> STOP. Mauvais axe ?"), FMath::Abs(RightAxis)));
		StopLateralMove();
	}
}

void ULedgeClimbingComponent::LedgeJump()
{
	if (CurrentState != ELedgeState::Hanging && CurrentState != ELedgeState::Climbing) return;

	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;
	
	const FVector LaunchVel =
		FVector::UpVector * LedgeJumpUpVelocity
		+ CurrentLedgeData.LedgeNormal * LedgeJumpOutVelocity;
	
	LateralDirection = 0.f;
	RestoreMovement();
	SetState(ELedgeState::None);
	CurrentLedgeData       = FLedgeData();
	bDetectionCooldown     = true;
	DetectionCooldownTimer = 0.15f;

	Char->LaunchCharacter(LaunchVel, /*bXYOverride=*/true, /*bZOverride=*/true);
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
	if (FMath::IsNearlyZero(LateralDirection))
	{
		SetState(ELedgeState::Hanging);
		return;
	}

	ACharacter* Char = GetOwnerCharacter();
	if (!Char) return;

	if (bDebugLedge && GEngine)
		GEngine->AddOnScreenDebugMessage(105, 0.5f, FColor::White, TEXT("[Lateral] TickClimbing actif"));

	const float StepDist = LateralMoveSpeed * DeltaTime;

	FLedgeData NewLedgeData;
	if (!CheckLedgeAtLateralOffset(LateralDirection * StepDist, NewLedgeData))
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(106, 1.f, FColor::Red,
				TEXT("[Lateral] CheckLedgeAtLateralOffset = FALSE -> arret"));
		LateralDirection = 0.f;
		SetState(ELedgeState::Hanging);
		return;
	}

	CurrentLedgeData = NewLedgeData;

	const FVector TargetPos  = ComputeHangPosition(CurrentLedgeData);
	const FVector CurrentPos = Char->GetActorLocation();
	const FVector NewPos     = FMath::VInterpTo(CurrentPos, TargetPos, DeltaTime, HangSnapSpeed);
	Char->SetActorLocation(NewPos, false);

	if (bDebugLedge && GEngine)
		GEngine->AddOnScreenDebugMessage(107, 0.5f, FColor::Green,
			FString::Printf(TEXT("[Lateral] OK, deplacement de %.1f cm"), FVector::Dist(CurrentPos, NewPos)));
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
		RestoreMovement();
		SetState(ELedgeState::None);
		CurrentLedgeData       = FLedgeData();
		bDetectionCooldown     = true;
		DetectionCooldownTimer = DetectionCooldownDuration;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Detection
// ─────────────────────────────────────────────────────────────────────────────

bool ULedgeClimbingComponent::DetectLedge(FLedgeData& OutLedgeData) const
{
	FHitResult WallHit;
	if (!TraceForWall(WallHit)) return false;
	
	FVector LedgeTopPos;
	if (!TraceForLedgeTop(WallHit, LedgeTopPos)) return false;

	const ACharacter* Char = GetOwnerCharacter();
	if (!Char) return false;
	
	const float CharFeetZ   = Char->GetActorLocation().Z - Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float LedgeDeltaZ = LedgeTopPos.Z - CharFeetZ;
	if (LedgeDeltaZ < MinLedgeHeight || LedgeDeltaZ > MaxLedgeHeight) return false;
	
	{
		FHitResult SurfaceHit;
		FCollisionQueryParams SurfaceParams;
		SurfaceParams.AddIgnoredActor(Char);
		const FVector CheckStart = LedgeTopPos + FVector(0.f, 0.f, 5.f);
		const FVector CheckEnd   = LedgeTopPos - FVector(0.f, 0.f, 5.f);
		if (GetWorld()->LineTraceSingleByChannel(SurfaceHit, CheckStart, CheckEnd, TraceChannel, SurfaceParams))
		{
			const float DotUp = FVector::DotProduct(SurfaceHit.ImpactNormal, FVector::UpVector);
			if (DotUp < MinSurfaceDotUp) return false;
		}
	}
	
	OutLedgeData.bIsValid         = true;
	OutLedgeData.LedgeNormal      = WallHit.ImpactNormal;
	OutLedgeData.LedgeTopPosition = LedgeTopPos;
	OutLedgeData.HangPosition     = ComputeHangPosition(OutLedgeData);
	
	OutLedgeData.bCanMantle = CanMantle(OutLedgeData, WallHit.GetActor());

	return true;
}

bool ULedgeClimbingComponent::TraceForWall(FHitResult& OutHit) const
{
	const ACharacter* Char = GetOwnerCharacter();
	if (!Char) return false;

	const FVector StartPos = Char->GetActorLocation();
	const FVector Forward  = Char->GetActorForwardVector();
	const FVector EndPos   = StartPos + Forward * WallDetectionDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Char);

	return GetWorld()->LineTraceSingleByChannel(OutHit, StartPos, EndPos, TraceChannel, Params);
}

bool ULedgeClimbingComponent::TraceForLedgeTop(const FHitResult& WallHit, FVector& OutLedgeTopPos) const
{
	const ACharacter* Char = GetOwnerCharacter();
	if (!Char) return false;
	return TraceForLedgeTopAt(WallHit, Char->GetActorLocation(), OutLedgeTopPos);
}

bool ULedgeClimbingComponent::TraceForLedgeTopAt(const FHitResult& WallHit,
                                                  const FVector& CharPos,
                                                  FVector& OutLedgeTopPos) const
{
	const FVector WallPoint  = WallHit.ImpactPoint;
	const FVector WallNormal = WallHit.ImpactNormal;

	const FVector TraceStart = FVector(
		WallPoint.X - WallNormal.X * LedgeDetectionRadius,
		WallPoint.Y - WallNormal.Y * LedgeDetectionRadius,
		CharPos.Z + MaxLedgeHeight);

	const FVector TraceEnd = FVector(TraceStart.X, TraceStart.Y, CharPos.Z - 10.f);

	FHitResult LedgeHit;
	FCollisionQueryParams Params;
	const ACharacter* Char = GetOwnerCharacter();
	if (Char) Params.AddIgnoredActor(Char);

	if (!GetWorld()->LineTraceSingleByChannel(LedgeHit, TraceStart, TraceEnd, TraceChannel, Params))
		return false;

	OutLedgeTopPos = LedgeHit.ImpactPoint;
	return true;
}

bool ULedgeClimbingComponent::IsLedgeClearAbove(const FVector& LedgeTopPos) const
{
	const ACharacter* Char = GetOwnerCharacter();
	if (!Char) return false;

	const float CapsuleHalfHeight = Char->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius     = Char->GetCapsuleComponent()->GetScaledCapsuleRadius();
	
	const float Clearance = 5.f;
	const FVector TestCenter = LedgeTopPos + FVector(0.f, 0.f, CapsuleHalfHeight + Clearance);

	const float TestRadius     = FMath::Max(CapsuleRadius - 5.f, 5.f);
	const float TestHalfHeight = FMath::Max(CapsuleHalfHeight - 5.f, TestRadius);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Char);

	const bool bBlocked = GetWorld()->OverlapBlockingTestByChannel(
		TestCenter, FQuat::Identity, TraceChannel,
		FCollisionShape::MakeCapsule(TestRadius, TestHalfHeight), Params);

	return !bBlocked;
}

bool ULedgeClimbingComponent::CanMantle(const FLedgeData& Ledge, const AActor* LedgeActor) const
{
	if (LedgeActor)
	{
		if (!HangOnlyTag.IsNone() && LedgeActor->ActorHasTag(HangOnlyTag))
			return false; 
		if (!ForceMantleTag.IsNone() && LedgeActor->ActorHasTag(ForceMantleTag))
			return true;   
	}

	const ACharacter* Char = GetOwnerCharacter();
	if (!Char) return false;
	
	if (!IsLedgeClearAbove(Ledge.LedgeTopPosition)) return false;
	
	const float CapsuleRadius = Char->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector StandXY = Ledge.LedgeTopPosition
		- Ledge.LedgeNormal * (CapsuleRadius + MantleDepthMargin);

	const FVector ProbeStart = FVector(StandXY.X, StandXY.Y, Ledge.LedgeTopPosition.Z + 20.f);
	const FVector ProbeEnd   = FVector(StandXY.X, StandXY.Y, Ledge.LedgeTopPosition.Z - 20.f);

	FHitResult FloorHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Char);

	if (!GetWorld()->LineTraceSingleByChannel(FloorHit, ProbeStart, ProbeEnd, TraceChannel, Params))
		return false; 

	if (FMath::Abs(FloorHit.ImpactPoint.Z - Ledge.LedgeTopPosition.Z) > 18.f)
		return false;

	return true;
}

bool ULedgeClimbingComponent::CheckLedgeAtLateralOffset(float LateralOffset, FLedgeData& OutLedgeData) const
{
	const ACharacter* Char = GetOwnerCharacter();
	if (!Char) return false;

	const FVector Normal     = CurrentLedgeData.LedgeNormal;
	const FVector LedgeRight  = FVector::CrossProduct(FVector::UpVector, Normal).GetSafeNormal();
	
	const FVector NewTopGuess = CurrentLedgeData.LedgeTopPosition + LedgeRight * LateralOffset;
	const FVector WallStart = NewTopGuess + Normal * 40.f + FVector(0.f, 0.f, -10.f);
	const FVector WallEnd   = NewTopGuess - Normal * 40.f + FVector(0.f, 0.f, -10.f);

	if (bDebugLedge)
	{
		DrawDebugLine(GetWorld(), WallStart, WallEnd, FColor::Magenta, false, 0.5f, 0, 2.f);
		DrawDebugPoint(GetWorld(), NewTopGuess, 12.f, FColor::Yellow, false, 0.5f);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(108, 1.f, FColor::White,
				FString::Printf(TEXT("[Check] Normal=(%.2f,%.2f,%.2f) Right=(%.2f,%.2f,%.2f) offset=%.2f"),
					Normal.X, Normal.Y, Normal.Z, LedgeRight.X, LedgeRight.Y, LedgeRight.Z, LateralOffset));
	}

	FHitResult WallHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Char);

	if (!GetWorld()->LineTraceSingleByChannel(WallHit, WallStart, WallEnd, TraceChannel, Params))
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(109, 1.f, FColor::Red,
				TEXT("[Check] ECHEC A : trace mur lateral ne touche rien"));
		return false;
	}
	if (bDebugLedge) DrawDebugSphere(GetWorld(), WallHit.ImpactPoint, 8.f, 8, FColor::Green, false, 0.5f);
	
	FVector NewLedgeTopPos;
	if (!TraceForLedgeTopAt(WallHit, Char->GetActorLocation(), NewLedgeTopPos))
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(110, 1.f, FColor::Red,
				TEXT("[Check] ECHEC B : pas de dessus de rebord a cote"));
		return false;
	}
	if (bDebugLedge) DrawDebugSphere(GetWorld(), NewLedgeTopPos, 8.f, 8, FColor::Blue, false, 0.5f);
	
	const float DeltaZ = FMath::Abs(NewLedgeTopPos.Z - CurrentLedgeData.LedgeTopPosition.Z);
	if (DeltaZ > LateralHeightTolerance)
	{
		if (bDebugLedge && GEngine)
			GEngine->AddOnScreenDebugMessage(111, 1.f, FColor::Red,
				FString::Printf(TEXT("[Check] ECHEC C : saut de hauteur %.1f > %.1f"), DeltaZ, LateralHeightTolerance));
		return false;
	}

	OutLedgeData.bIsValid         = true;
	OutLedgeData.LedgeNormal      = WallHit.ImpactNormal;
	OutLedgeData.LedgeTopPosition = NewLedgeTopPos;
	OutLedgeData.HangPosition     = ComputeHangPosition(OutLedgeData);
	OutLedgeData.bCanMantle       = CanMantle(OutLedgeData, WallHit.GetActor());

	return true;
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
	const float ForwardOffset = LedgeDetectionRadius + CapsuleRadius + HangWallGap;

	return LedgeData.LedgeTopPosition
		+ LedgeData.LedgeNormal * ForwardOffset
		+ FVector(0.f, 0.f, -CapsuleHalfHeight - HangDropOffset);
}

ACharacter* ULedgeClimbingComponent::GetOwnerCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}