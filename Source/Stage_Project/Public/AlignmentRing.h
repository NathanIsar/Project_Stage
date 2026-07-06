#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AlignmentRing.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRingAligned, AAlignmentRing*, Ring);

UCLASS()
class STAGE_PROJECT_API AAlignmentRing : public AActor
{
    GENERATED_BODY()

public:
    AAlignmentRing();

    UPROPERTY(BlueprintAssignable, Category = "Ring")
    FOnRingAligned OnRingAligned;

    UFUNCTION(BlueprintPure, Category = "Ring")
    bool IsSolved() const { return bSolved; }

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaSeconds) override;

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* Root;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* RingMesh;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UStaticMeshComponent* SymbolDisk;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USceneComponent* SymbolTarget;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Alignment", meta = (ClampMin = "1.0"))
    float HoleRadius = 25.f;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Alignment", meta = (ClampMin = "0.5"))
    float SymbolRadius = 8.f;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Alignment", meta = (ClampMin = "0.0"))
    float HoldTime = 0.35f;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Alignment", meta = (ClampMin = "0.0"))
    float MaxViewDistance = 0.f;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Alignment")
    bool bRequireLineOfSight = true;

    UPROPERTY(EditAnywhere, Category = "Ring|Fade", meta = (ClampMin = "0.01"))
    float FadeDuration = 0.6f;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Feedback")
    UMaterialInterface* SymbolMaterial = nullptr;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Feedback")
    USoundBase* SolveSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Ring|Feedback", meta = (ClampMin = "0.0"))
    float SolveVolume = 1.f;
    
    UPROPERTY(EditAnywhere, Category = "Ring|Fade")
    FName DissolveParamName = TEXT("Dither_Opacity");

private:
    bool IsPlayerAligned() const;
    void StartFade();

    UPROPERTY()
    UMaterialInstanceDynamic* DiskMID = nullptr;

    bool bSolved = false;
    float AlignHold = 0.f;
    bool bFading = false;
    float FadeElapsed = 0.f;
};