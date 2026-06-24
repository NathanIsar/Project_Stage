#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "TrajectoryPreviewActor.h"        
#include "GrabComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STAGE_PROJECT_API UGrabComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGrabComponent();

	// ─── Config : Grab ──────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	float ReachDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	float HoldDistance = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	FName HoldSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Config")
	float HoverCheckInterval = 0.1f;

	// ─── Config : Throw ─────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Throw")
	float ThrowForce = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Throw")
	float TrajectoryMaxTime = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Throw")
	float TrajectorySimFrequency = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Throw")
	float TrajectoryProjectileRadius = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Throw")
	float TrajectoryRefreshRate = 0.05f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Grab|Throw")
	TSubclassOf<ATrajectoryPreviewActor> PreviewActorClass;

	// ─── État ───────────────────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category="Grab|State")
	AActor* HeldActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Grab|State")
	AActor* HoveredActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Grab|State")
	bool bIsThrowReady = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Grab|Throw")
	ATrajectoryPreviewActor* PreviewActor = nullptr;

	// ─── Données trajectoire ────────────────────────────────────────────────────

	UPROPERTY(BlueprintReadOnly, Category="Grab|Throw")
	TArray<FVector> ThrowTrajectoryPoints;

	UPROPERTY(BlueprintReadOnly, Category="Grab|Throw")
	FVector ThrowLandingPoint = FVector::ZeroVector;

	// ─── Actions : Clic Gauche ──────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category="Grab")
	void TryGrabOrRelease();

	UFUNCTION(BlueprintCallable, Category="Grab")
	void Release();

	// ─── Actions : Clic Droit ───────────────────────────────────────────────────
	
	UFUNCTION(BlueprintCallable, Category="Grab|Throw")
	void StartThrowAim();
	
	UFUNCTION(BlueprintCallable, Category="Grab|Throw")
	void ReleaseThrow();
	
	UFUNCTION(BlueprintCallable, Category="Grab|Throw")
	void CancelThrowAim();

	UFUNCTION(BlueprintCallable, Category="Grab")
	void UpdateLookAt();

	// ─── Delegates ──────────────────────────────────────────────────────────────

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabChanged, AActor*, Actor);

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnGrabChanged OnActorGrabbed;

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnGrabChanged OnActorReleased;

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnGrabChanged OnActorThrown;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHoverChanged, AActor*, OldActor, AActor*, NewActor);

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnHoverChanged OnHoverChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThrowReadyChanged, bool, bReady);

	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnThrowReadyChanged OnThrowReadyChanged;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrajectoryUpdated, const TArray<FVector>&, Points);
	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnTrajectoryUpdated OnTrajectoryUpdated;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrajectoryHidden);
	UPROPERTY(BlueprintAssignable, Category="Grab|Events")
	FOnTrajectoryHidden OnTrajectoryHidden;

	// ─── Sons ────────────────────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grab|Sound")
	USoundBase* GrabSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grab|Sound")
	USoundBase* ReleaseSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grab|Sound")
	USoundBase* ThrowSound = nullptr;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Grab|Sound")
	UAudioComponent* SlotAudioComponent = nullptr;

public:
	UPROPERTY()
	UCameraComponent* PlayerCamera = nullptr;

	bool bWasSimulatingPhysics = false;
	ECollisionResponse SavedPawnResponse = ECR_Block;

	AActor* LineTraceForGrabbable() const;
	void    Grab(AActor* Target);
	void    SetHoveredActor(AActor* NewHovered);

private:
	FTimerHandle HoverTimerHandle;
	FTimerHandle TrajectoryTimerHandle;

	UFUNCTION()
	void HoverTimerTick();

	void UpdateTrajectory();
	void HideTrajectory();
	void ExecuteThrow();
};