#pragma once

#include "CoreMinimal.h"
#include "BaseGrabbableActor.h"
#include "NumberGrab.generated.h"

UCLASS()
class STAGE_PROJECT_API ANumberGrab : public ABaseGrabbableActor
{
	GENERATED_BODY()

public:
	ANumberGrab();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Puzzle|Digit")
	int32 DigitValue = 0;

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
	virtual FText GetActionName_Implementation() const override;
};