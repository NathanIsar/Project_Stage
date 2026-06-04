#pragma once
 
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "InteractionComponent.generated.h"
 
 
class IIInteractable;
class USphereComponent;
 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableDetected, AActor*, InteractableActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractableLost);
 
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STAGE_PROJECT_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()
 
public:	
 
	UInteractionComponent();
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void TryInteract();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractable() const { return CurrentInteractable; }
	
	UPROPERTY(BlueprintReadOnly, Category="Grab|State")
	AActor* FocusedInteractable = nullptr;
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Interaction")
	const TArray<AActor*>& GetInteractablesInRange() const { return InteractablesInRange; }
	
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableDetected OnInteractableDetected;
 
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableLost OnInteractableLost;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);
 
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
 
 
protected:
 
	virtual void BeginPlay() override;
	
	virtual void OnRegister() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionRange = 300.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float GlobalCooldown = 0.5f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bDebugMode = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float HUDRange = 180.f;
	
	FTimerHandle HUDUpdateTimerHandle;
	void UpdateHUDVisibility();
 
	UPROPERTY()
	UCameraComponent* PlayerCamera;
 
 
private:	
	
	UPROPERTY()
	TObjectPtr<USphereComponent> InteractionSphere;
	
	UPROPERTY()
	TObjectPtr<AActor> CurrentInteractable;
	
	UPROPERTY()
	TArray<TObjectPtr<AActor>> InteractablesInRange;
	
	FTimerHandle CooldownTimerHandle;
	bool bCanInteract = true;
	
	AActor* FindBestInteractable();
	
	void ResetCooldown();
	
	void UpdateCurrentInteractable(AActor* NewInteractable);
};
 