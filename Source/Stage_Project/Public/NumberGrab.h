#pragma once

#include "CoreMinimal.h"
#include "IGrabbable.h"
#include "GameFramework/Actor.h"
#include "NumberGrab.generated.h"

UCLASS()
class STAGE_PROJECT_API ANumberGrab : public AActor, public IIGrabbable
{
	GENERATED_BODY()
	
public:	
	ANumberGrab();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Digit")
	int32 DigitValue = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="Puzzle|Digit")
	bool bIsGrabbed = false;
 
	UPROPERTY(BlueprintReadOnly, Category="Puzzle|Digit")
	bool bIsPlaced = false;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDigitStateChanged);
 
	UPROPERTY(BlueprintAssignable, Category="Puzzle|Events")
	FOnDigitStateChanged OnDigitGrabbed;
 
	UPROPERTY(BlueprintAssignable, Category="Puzzle|Events")
	FOnDigitStateChanged OnDigitReleased;
 
	UPROPERTY(BlueprintAssignable, Category="Puzzle|Events")
	FOnDigitStateChanged OnDigitHovered;
 
	UPROPERTY(BlueprintAssignable, Category="Puzzle|Events")
	FOnDigitStateChanged OnDigitUnhovered;
	
	virtual void OnGrabbed_Implementation(AActor* Grabber) override;
	virtual void OnReleased_Implementation(FVector DropLocation) override;
	virtual void OnHovered_Implementation() override;
	virtual void OnUnhovered_Implementation() override;
	virtual FText GetInteractLabel_Implementation() override;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Digit")
	UMaterialInterface* DefaultMaterial = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Digit")
	UMaterialInterface* GrabbedMaterial = nullptr;
};
