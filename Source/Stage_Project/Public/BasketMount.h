// BasketMount.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasketMount.generated.h"

class UStaticMeshComponent;
class ABasketMount;

USTRUCT(BlueprintType)
struct FGearConnection
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gear")
	TObjectPtr<ABasketMount> ConnectedBasket = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gear", meta = (ClampMin = "0.01"))
	float GearRatio = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gear")
	bool bInvertDirection = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBasketRotationComplete, ABasketMount*, Basket);

UCLASS(Blueprintable)
class STAGE_PROJECT_API ABasketMount : public AActor
{
	GENERATED_BODY()

public:
	ABasketMount();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Basket")
	TObjectPtr<UStaticMeshComponent> BasketMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basket|Rotation")
	float RotationDuration = 0.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basket|Rotation")
	bool bQueueRotationsWhileBusy = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basket|Gear")
	TArray<FGearConnection> GearConnections;

	UPROPERTY(BlueprintAssignable, Category = "Basket")
	FOnBasketRotationComplete OnRotationComplete;

	UFUNCTION(BlueprintPure, Category = "Basket")
	bool IsRotating() const { return bIsRotating; }
	
	UFUNCTION(BlueprintCallable, Category = "Basket|Rotation")
	void AddRotationDelta(const FRotator& Delta);

	UFUNCTION(BlueprintCallable, Category = "Basket|Gear")
	void PropagateRotation(const FRotator& Delta, UPARAM(ref) TArray<ABasketMount*>& VisitedBaskets);
	
	UFUNCTION(BlueprintPure, Category = "Basket")
	bool IsOpenFaceFacing(const FVector& WorldDirection, float ToleranceDegrees = 15.0f) const;

private:
	bool bIsRotating = false;
	float ElapsedTime = 0.0f;
	FRotator StartRotation;
	FRotator TargetRotation;

	TArray<FRotator> PendingDeltas;
};