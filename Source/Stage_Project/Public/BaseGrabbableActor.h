#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IGrabbable.h"
#include "BaseGrabbableActor.generated.h"


UCLASS(Abstract, Blueprintable)
class STAGE_PROJECT_API ABaseGrabbableActor : public AActor, public IIGrabbable
{
	GENERATED_BODY()

public:
	ABaseGrabbableActor();
	
	virtual void OnGrabbed_Implementation(AActor* Grabber) override;
	virtual void OnReleased_Implementation(FVector DropLocation) override;
	virtual void OnHovered_Implementation() override;
	virtual void OnUnhovered_Implementation() override;
	virtual FText GetActionName_Implementation() const override;


	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	bool bIsGrabbed = false;

	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	bool bIsHovered = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	AActor* CurrentGrabber = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Grab|State")
	FVector LastDropLocation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Settings")
	FText InteractLabel = FText::FromString(TEXT("Grab"));
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Settings")
	bool bSimulatePhysicsWhileGrabbed = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Settings")
	bool bDisableCollisionWhileGrabbed = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab|Debug")
	bool bDebugMode = false;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab|Components")
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Materials")
	UMaterialInterface* DefaultMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Materials")
	UMaterialInterface* HoveredMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Materials")
	UMaterialInterface* GrabbedMaterial = nullptr;

protected:
	virtual void BeginPlay() override;
	

	UFUNCTION(BlueprintImplementableEvent, Category = "Grab|Events")
	void OnGrabbedEvent(AActor* Grabber);

	UFUNCTION(BlueprintImplementableEvent, Category = "Grab|Events")
	void OnReleasedEvent(FVector DropLocation);

	UFUNCTION(BlueprintImplementableEvent, Category = "Grab|Events")
	void OnHoveredEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Grab|Events")
	void OnUnhoveredEvent();

private:
	void ApplyMaterial(UMaterialInterface* Material);
};