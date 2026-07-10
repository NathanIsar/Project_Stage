#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EndGameTrigger.generated.h"

class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndReached);

UCLASS()
class STAGE_PROJECT_API AEndGameTrigger : public AActor
{
	GENERATED_BODY()

public:
	AEndGameTrigger();

	UPROPERTY(BlueprintAssignable, Category = "EndGame")
	FOnEndReached OnEndReached;

protected:
	UPROPERTY(VisibleAnywhere, Category = "EndGame")
	TObjectPtr<UBoxComponent> TriggerBox;
	
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
					   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
					   bool bFromSweep, const FHitResult& SweepResult);

private:
	bool bTriggered = false;
};