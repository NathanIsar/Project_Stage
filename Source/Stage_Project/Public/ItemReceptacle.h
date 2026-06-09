#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ItemReceptacle.generated.h"

UCLASS()
class STAGE_PROJECT_API AItemReceptacle : public AActor
{
	GENERATED_BODY()

public:
	AItemReceptacle();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* DetectionBox;
	UPROPERTY(EditAnywhere, Category = "Receptacle")
	TSubclassOf<AActor> RequiredItemClass;
	
	UPROPERTY(EditAnywhere, Category = "Receptacle")
	class APuzzleDoor* LinkedDoor;

	bool bAlreadyTriggered;
};