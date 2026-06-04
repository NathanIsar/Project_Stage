#pragma once

#include "CoreMinimal.h"
#include "BaseInteractableActor.h"
#include "Components/BoxComponent.h"
#include "DoorActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoorManuallyClosed);
UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Closed UMETA(DisplayName = "Closed"),
	Opening UMETA(DisplayName = "Opening"),
	Open UMETA(DisplayName = "Open"),
	Closing UMETA(DisplayName = "Closing")
};

UCLASS()
class STAGE_PROJECT_API ADoorActor : public ABaseInteractableActor
{
	GENERATED_BODY()
	
	public:
    ADoorActor();

    // Plus d'interaction manuelle — la porte se ferme quand le joueur la traverse
    virtual bool CanInteract_Implementation() const override { return false; }

    UFUNCTION(BlueprintCallable, Category = "Door")
    void UnlockAndOpen();
    
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Door")
    EDoorState GetDoorState() const { return DoorState; }

    // Broadcasté quand le joueur traverse la porte (elle se referme)
    UPROPERTY(BlueprintAssignable, Category = "Door")
    FOnDoorManuallyClosed OnDoorManuallyClosed;

protected:
    virtual void BeginPlay() override;

    // Door specific settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    EDoorState DoorState = EDoorState::Closed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenAngle = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenDuration = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bRequiresKey = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (EditCondition = "bRequiresKey"))
    FName RequiredKeyID = "DefaultKey";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool bAutoClose = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (EditCondition = "bAutoClose"))
    float AutoCloseDelay = 3.f;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    class UStaticMeshComponent* DoorFrame;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    class UStaticMeshComponent* DoorMesh;

    // Zone de détection — quand le joueur la traverse, la porte se referme
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    UBoxComponent* PlayerTrigger;

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Door")
    void OnDoorOpened();

    UFUNCTION(BlueprintImplementableEvent, Category = "Door")
    void OnDoorClosed();

    UFUNCTION(BlueprintImplementableEvent, Category = "Door")
    void OnDoorLocked();
    
    // Sons
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
    class USoundBase* OpenSound;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Audio")
    class USoundBase* CloseSound;

private:
    FTimerHandle AutoCloseTimerHandle;
    FTimerHandle AnimationTimerHandle;  
    FRotator ClosedRotation;
    FRotator OpenRotation;
    
    FTimerHandle LerpTimerHandle;
    float LerpAlpha;
    FRotator StartRotation;
    FRotator TargetRotation;

    UFUNCTION()
    void OpenDoor();

    UFUNCTION()
    void CloseDoor();
	
    UFUNCTION()
    void OnPlayerEnterDoor(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);
    
    void UpdateOpenAnimation();
    void UpdateCloseAnimation();

    void ScheduleAutoClose();
};