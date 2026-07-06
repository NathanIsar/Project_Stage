#pragma once

#include "CoreMinimal.h"
#include "BaseInteractableActor.h"
#include "RotatingStatueActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatueOrientationChanged, bool, bIsCorrectOrientation);

UCLASS()
class STAGE_PROJECT_API ARotatingStatueActor : public ABaseInteractableActor
{
	GENERATED_BODY()

public:
	ARotatingStatueActor();

	virtual void Interact_Implementation(AActor* Interactor) override;
	virtual FText GetActionName_Implementation() const override;

	UPROPERTY(BlueprintAssignable, Category = "Statue")
	FOnStatueOrientationChanged OnOrientationChanged;

	UFUNCTION(BlueprintPure, Category = "Statue")
	bool IsCorrectOrientation() const { return bIsCorrectOrientation; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Components")
	UStaticMeshComponent* SocleMeshComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Config")
	float TargetYawWorld = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Config")
	float OrientationTolerance = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Config")
	float RotationStepDegrees = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Config")
	float RotationDuration = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Feedback")
	int32 IncorrectStencilValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Feedback")
	int32 CorrectStencilValue = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statue|Feedback")
	class USoundBase* RotationSound;

private:
	FTimerHandle RotationTimerHandle;
	float RotationElapsedTime = 0.f;
	float RotationStartYaw = 0.f;
	float RotationTargetYaw = 0.f;
	bool bIsRotating = false;
	bool bIsCorrectOrientation = false;

	static constexpr float RotationTickInterval = 0.016f;

	void TickRotation();
	void UpdateOrientationState();
	float GetShortestYawDelta(float From, float To) const;
};