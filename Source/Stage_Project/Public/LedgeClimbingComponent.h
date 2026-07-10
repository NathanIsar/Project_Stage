#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LedgeClimbingComponent.generated.h"

class ULedgeMarkerComponent;

UENUM(BlueprintType)
enum class ELedgeState : uint8
{
	None		UMETA(DisplayName = "None"),
	Detected	UMETA(DisplayName = "Detected"),
	Hanging		UMETA(DisplayName = "Hanging"),
	Climbing	UMETA(DisplayName = "Climbing"),
	Vaulting	UMETA(DisplayName = "Vaulting")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLedgeDetected, ULedgeMarkerComponent*, LedgeMarker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLedgeLost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLedgeStateChanged, ELedgeState, NewState);

USTRUCT(BlueprintType)
struct FLedgeData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FVector LedgeTopPosition = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector LedgeNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector HangPosition = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly)
	bool bCanMantle = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STAGE_PROJECT_API ULedgeClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULedgeClimbingComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void TryGrabLedge();
	
	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void StartClimbDown();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void TryClimbUp();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void ReleaseLedge();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void StartLateralMove(float Direction);

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void StopLateralMove();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void UpdateLateralInput(float RightAxis);

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void LedgeJump();
	
	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void DropToLowerLedge();

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	ELedgeState GetLedgeState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	const FLedgeData& GetLedgeData() const { return CurrentLedgeData; }

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	bool IsHanging() const { return CurrentState == ELedgeState::Hanging; }

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	bool IsClimbing() const { return CurrentState == ELedgeState::Climbing || CurrentState == ELedgeState::Vaulting; }

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	bool CanMantleCurrentLedge() const { return CurrentLedgeData.bCanMantle; }
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	float GetLateralDirection() const { return LateralDirection; }
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|IK")
	FVector GetLeftHandTarget() const;
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|IK")
	FVector GetRightHandTarget() const;
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|IK")
	FRotator GetLeftHandRotation() const;
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|IK")
	FRotator GetRightHandRotation() const;
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|IK")
	bool IsOnLedge() const { return CurrentState == ELedgeState::Hanging || CurrentState == ELedgeState::Climbing; }

	// ── Détection ───────────────────────────────────────────────────────────
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float GrabRange = 120.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float MaxDropDistance = 200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float VerticalSeparation = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Debug")
	bool bDebugLedge = false;

	// ── Mouvement ───────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangSnapSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float VaultDuration = 0.6f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangDropOffset = 110.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangForwardOffset = 25.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float CameraBlendDuration = 0.25f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|IK")
	float HandSpacing = 40.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|IK")
	FRotator HandAlignmentRight = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|IK")
	FRotator HandAlignmentLeft = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float MaxLookYaw = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float LateralMoveSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float LedgeJumpUpVelocity = 550.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float LedgeJumpOutVelocity = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float MantleInputLockoutTime = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float AutoGrabWindow = 1.2f;

	// ── Animation ───────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> ClimbMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> VaultMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> GrabMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> ReleaseToGroundMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> DropMontage = nullptr;
	
	// HUD
	
	UPROPERTY(BlueprintAssignable, Category = "Ledge Climbing|Events")
	FOnLedgeDetected OnLedgeDetected;

	UPROPERTY(BlueprintAssignable, Category = "Ledge Climbing|Events")
	FOnLedgeLost OnLedgeLost;

	UPROPERTY(BlueprintAssignable, Category = "Ledge Climbing|Events")
	FOnLedgeStateChanged OnLedgeStateChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Ledge Climbing|Detection")
	TObjectPtr<ULedgeMarkerComponent> FocusedLedge = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float LedgeLookAngle = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float DetectionScanInterval = 0.1f;
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|HUD")
	bool ShouldShowGrabPrompt() const { return FocusedLedge != nullptr; }
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|HUD")
	bool ShouldShowJumpPrompt() const
	{
		return (CurrentState == ELedgeState::Hanging || CurrentState == ELedgeState::Climbing)
			&& !CurrentLedgeData.bCanMantle;
	}
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|HUD")
	bool ShouldShowMantlePrompt() const
	{
		return (CurrentState == ELedgeState::Hanging || CurrentState == ELedgeState::Climbing)
			&& CurrentLedgeData.bCanMantle;
	}
	
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|HUD")
	bool ShouldShowDropPrompt() const
	{
		return CurrentState == ELedgeState::Hanging || CurrentState == ELedgeState::Climbing;
	}

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing|HUD")
	bool ShouldShowLedgeHUD() const
	{
		return ShouldShowGrabPrompt() || ShouldShowMantlePrompt() || ShouldShowDropPrompt();
	}

private:

	ULedgeMarkerComponent* FindNearestLedge(const FVector& From, float Range, int32 VertDir,
	                                        float RefZ, ULedgeMarkerComponent* Skip, FVector& OutPoint) const;
	void AttachToLedge(ULedgeMarkerComponent* Marker, const FVector& Point);

	void SetState(ELedgeState NewState);
	void TickHanging(float DeltaTime);
	void TickClimbing(float DeltaTime);
	void TickVaulting(float DeltaTime);

	ACharacter* GetOwnerCharacter() const;
	void PlayLedgeMontage(UAnimMontage* Montage);
	void ApplyHangingPhysics();
	void RestoreMovement();
	FVector ComputeHangPosition(const FLedgeData& LedgeData) const;

	ELedgeState CurrentState = ELedgeState::None;
	FLedgeData  CurrentLedgeData;

	TWeakObjectPtr<ULedgeMarkerComponent> CurrentLedge;
	FVector CurrentLedgePoint = FVector::ZeroVector;

	float    VaultTimer         = 0.f;
	FVector  VaultStartLocation = FVector::ZeroVector;
	FVector  VaultEndLocation   = FVector::ZeroVector;

	float LateralDirection = 0.f;

	float GrabTimeSeconds = -100.f;

	bool  bAutoGrabbing  = false;
	float AutoGrabTimer  = 0.f;
	float JumpFromLedgeZ = 0.f;

	float SavedMaxFlySpeed  = 600.f;
	float SavedMaxWalkSpeed = 600.f;
	bool  bInputDisabled    = false;
	
	bool  bSavedUseControllerYaw = false;
	bool  bSavedOrientToMovement = false;
	
	bool     bCameraBlending   = false;
	float    CameraBlendTimer  = 0.f;
	FRotator CameraBlendStart  = FRotator::ZeroRotator;
	FRotator CameraBlendTarget = FRotator::ZeroRotator;

	bool  bDetectionCooldown     = false;
	float DetectionCooldownTimer = 0.f;
	static constexpr float DetectionCooldownDuration = 0.4f;
	
	
	void UpdateLedgeDetection(float DeltaTime);
	bool IsLedgeInView(const FVector& Point) const;

	float DetectionScanTimer = 0.f;
};