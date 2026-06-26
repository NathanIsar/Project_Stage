#pragma once

#include "CoreMinimal.h"
#include "BaseInteractableActor.h"
#include "ObjectDispenser.generated.h"

class ABaseGrabbableActor;

UCLASS()
class STAGE_PROJECT_API AObjectDispenser : public ABaseInteractableActor
{
	GENERATED_BODY()

public:
	AObjectDispenser();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetActionName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispenser")
	TSubclassOf<ABaseGrabbableActor> ObjectToDispense;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispenser")
	FVector SpawnOffset = FVector(-100.f, 0.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dispenser")
	int32 MaxActiveObjects = 1;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Dispenser")
	void OnObjectDispensed(ABaseGrabbableActor* SpawnedObject);

private:
	UPROPERTY()
	TArray<ABaseGrabbableActor*> ActiveObjects;

	void CleanupDestroyedObjects();
};