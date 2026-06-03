#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "BaseInteractableActor.generated.h"

class IIInteractable;

UCLASS()
class STAGE_PROJECT_API ABaseInteractableActor : public AActor, public IIInteractable
{
	GENERATED_BODY()
	
public:	
	ABaseInteractableActor();
	
	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual bool CanInteract_Implementation() const override;
	virtual void OnInteractionRangeEntered_Implementation(AActor* Interactor) override;
	virtual void OnInteractionRangeExited_Implementation(AActor* Interactor) override;
	virtual FText GetActionName_Implementation() const override;

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Settings")
	bool bIsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Settings")
	float InteractionCooldown = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Settings")
	bool bRequiresLineOfSight = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Feedback")
	class UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction|Debug")
	bool bDebugMode = false;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnInteracted(AActor* Interactor);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnPlayerEnterRange(AActor* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void OnPlayerExitRange(AActor* Player);

private:	

	FTimerHandle CooldownTimerHandle;
	
};