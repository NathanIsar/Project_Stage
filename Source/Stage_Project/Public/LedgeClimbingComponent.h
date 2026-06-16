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

	/** Descend vers le rebord le plus proche EN DESSOUS. Si aucun, lâche vers le sol.
	 *  À brancher sur une touche dédiée (ex. Crouch/Ctrl) en "Started". */
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

	/** -1 (gauche), 0, +1 (droite). Pour le blend space du shimmy (à lisser côté AnimBP). */
	UFUNCTION(BlueprintPure, Category = "Ledge Climbing")
	float GetLateralDirection() const { return LateralDirection; }

	// ── Détection ───────────────────────────────────────────────────────────

	/** Rayon autour du perso pour trouver un rebord à saisir (cm). Plus grand = plus tolérant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float GrabRange = 120.f;

	/** Distance max pour trouver un rebord en contrebas lors d'un drop (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float MaxDropDistance = 200.f;

	/** Écart vertical min pour qu'un rebord compte comme "au-dessus"/"en-dessous" (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Detection")
	float VerticalSeparation = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Debug")
	bool bDebugLedge = false;

	// ── Mouvement ───────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangSnapSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float VaultDuration = 0.6f;

	/** Distance verticale entre le bord saisi et le centre de la capsule en accroche. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangDropOffset = 80.f;

	/** Jeu entre la capsule et le mur en accroche (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float HangWallGap = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float LateralMoveSpeed = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float LedgeJumpUpVelocity = 550.f;

	/** Garde PETIT (~30-50) pour rester près du mur et que l'auto-grab marche. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float LedgeJumpOutVelocity = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float MantleInputLockoutTime = 0.2f;

	/** Durée d'auto-grab du rebord supérieur après un saut (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Movement")
	float AutoGrabWindow = 1.2f;

	// ── Animation ───────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> ClimbMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> VaultMontage = nullptr;

	/** Idle_To_Braced_Hang */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> GrabMontage = nullptr;

	/** Braced_Hang_Drop__2 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> ReleaseToGroundMontage = nullptr;

	/** Braced_Hang_Drop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Climbing|Animation")
	TObjectPtr<UAnimMontage> DropMontage = nullptr;

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

	bool  bDetectionCooldown     = false;
	float DetectionCooldownTimer = 0.f;
	static constexpr float DetectionCooldownDuration = 0.4f;
};