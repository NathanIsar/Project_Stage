#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LedgeClimbingComponent.generated.h"

UENUM(BlueprintType)
enum class ELedgeState : uint8
{
	None		UMETA(DisplayName = "None"),
	Detected	UMETA(DisplayName = "Detected"),
	Hanging		UMETA(DisplayName = "Hanging"),
	Climbing	UMETA(DisplayName = "Climbing"),
	Vaulting	UMETA(DisplayName = "Vaulting")
};

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
	void TryClimbUp();
	
	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void ReleaseLedge();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void StartLateralMove(float Direction);

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void StopLateralMove();

	UFUNCTION(BlueprintCallable, Category = "Ledge Climbing")
	void UpdateLateralInput(float RightAxis);

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	ELedgeState GetLedgeState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	const FLedgeData& GetLedgeData() const { return CurrentLedgeData; }

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	bool IsHanging() const { return CurrentState == ELedgeState::Hanging; }

	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	bool IsClimbing() const { return CurrentState == ELedgeState::Climbing || CurrentState == ELedgeState::Vaulting; }

	// ── Detection ─────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float WallDetectionDistance = 80.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float MaxLedgeHeight = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float MinLedgeHeight = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float LedgeDetectionRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float MinSurfaceDotUp = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// ── Movement ──────────────────────────────────────────────────────────────
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangSnapSpeed = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float VaultDuration = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangDropOffset = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float LateralMoveSpeed = 150.f;
	
	// ── Animation ─────────────────────────────────────────────────────────────
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> ClimbMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> VaultMontage = nullptr;

private:

	// ── Détection ─────────────────────────────────────────────────────────────
	bool DetectLedge(FLedgeData& OutLedgeData) const;
	bool TraceForWall(FHitResult& OutHit) const;

	bool TraceForLedgeTop(const FHitResult& WallHit, FVector& OutLedgeTopPos) const;


	bool TraceForLedgeTopAt(const FHitResult& WallHit, const FVector& CharPos, FVector& OutLedgeTopPos) const;

	bool IsLedgeClearAbove(const FVector& LedgeTopPos) const;

	bool CheckLedgeAtLateralOffset(float LateralOffset, FLedgeData& OutLedgeData) const;
	
	// ── State machine ─────────────────────────────────────────────────────────
	void SetState(ELedgeState NewState);
	void TickHanging(float DeltaTime);
	void TickClimbing(float DeltaTime);
	void TickVaulting(float DeltaTime);
	
	// ── Helpers ───────────────────────────────────────────────────────────────
	ACharacter* GetOwnerCharacter() const;
	void ApplyHangingPhysics();
	void RestoreMovement();
	FVector ComputeHangPosition(const FLedgeData& LedgeData) const;
	
	// ── State ─────────────────────────────────────────────────────────────────
	ELedgeState CurrentState = ELedgeState::None;
	FLedgeData  CurrentLedgeData;

	float    VaultTimer         = 0.f;
	FVector  VaultStartLocation = FVector::ZeroVector;
	FVector  VaultEndLocation   = FVector::ZeroVector;
	FRotator VaultStartRotation = FRotator::ZeroRotator;

	float LateralDirection = 0.f;

	float SavedMaxFlySpeed  = 600.f;
	float SavedMaxWalkSpeed = 600.f;
	bool  bInputDisabled    = false;

	bool  bDetectionCooldown     = false;
	float DetectionCooldownTimer = 0.f;
	static constexpr float DetectionCooldownDuration = 0.5f;
};