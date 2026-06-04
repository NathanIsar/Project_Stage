// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseInteractableActor.h"
#include "SteleActor.generated.h"


UENUM()
enum class ESymbolType : uint8
{
	Sphere UMETA(DisplayName = "Sphere"),
	Cone UMETA(DisplayName = "Cone"),
	Cube UMETA(DisplayName = "Cube"),
	Cylinder UMETA(DisplayName = "Cylinder"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSteleRotated, ESymbolType, NewFrontSymbol);


UCLASS()
class STAGE_PROJECT_API ASteleActor : public ABaseInteractableActor
{
	GENERATED_BODY()
	
	public:
    ASteleActor();

    virtual void Interact_Implementation(AActor* Interactor) override;
    virtual bool CanInteract_Implementation() const override;
	
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Stele")
    ESymbolType GetCurrentFrontSymbol() const;
	
    UFUNCTION(BlueprintCallable, Category = "Stele")
    void ResetStele();
	
    UFUNCTION(BlueprintCallable, Category = "Stele")
    void SetToSymbol(ESymbolType TargetSymbol);
	
    UPROPERTY(BlueprintAssignable, Category = "Stele")
    FOnSteleRotated OnSteleRotated;

protected:
    virtual void BeginPlay() override;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Symbols")
    ESymbolType FrontSymbol = ESymbolType::Sphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Symbols")
    ESymbolType RightSymbol = ESymbolType::Cone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Symbols")
    ESymbolType BackSymbol = ESymbolType::Cube;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Symbols")
    ESymbolType LeftSymbol = ESymbolType::Cylinder;

    // Animation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Animation")
    float RotationDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Animation")
    bool bClockwiseRotation = true;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Animation")
    bool bRotateBasedOnPlayerSide = true;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stele")
    class UStaticMeshComponent* SteleMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stele")
    class UStaticMeshComponent* BaseMesh;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Materials")
    TMap<ESymbolType, class UMaterialInterface*> SymbolMaterials;

    // Sons
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Audio")
    class USoundBase* RotationSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stele|Audio")
    class USoundBase* RotationCompleteSound;
	
    UFUNCTION(BlueprintImplementableEvent, Category = "Stele")
    void OnRotationStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Stele")
    void OnRotationCompleted(ESymbolType NewFrontSymbol);

    UFUNCTION(BlueprintImplementableEvent, Category = "Stele")
    void OnSymbolChanged(ESymbolType OldSymbol, ESymbolType NewSymbol);

private:
	
    UPROPERTY()
    int32 CurrentRotationIndex = 0;
	
    UPROPERTY()
    bool bIsRotating = false;
	
    FTimerHandle RotationTimerHandle;
	
    FRotator StartRotation;
    FRotator TargetRotation;
	
    void StartRotation2(bool bClockwise);
    void UpdateRotation();
    void CompleteRotation();
	
    ESymbolType GetSymbolAtIndex(int32 Index) const;
	
    bool ShouldRotateClockwise(AActor* Interactor) const;
	
    float RotationElapsedTime = 0.f;
};