#include "PuzzleStele.h"
#include "PuzzleDoor.h"
#include "Kismet/GameplayStatics.h"

APuzzleStele::APuzzleStele()
{
	PrimaryActorTick.bCanEverTick = false;
	
	UStaticMeshComponent* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SteleMesh"));
	RootComponent = Mesh;
}
 
void APuzzleStele::BeginPlay()
{
	Super::BeginPlay();
	InitSlots();
}
 
void APuzzleStele::InitSlots()
{
	Slots.Empty();
	GetComponents<UPuzzleSlotComponent>(Slots);
	
	Slots.Sort([](const UPuzzleSlotComponent& A, const UPuzzleSlotComponent& B)
	{
		return A.SlotIndex < B.SlotIndex;
	});
	
	for (UPuzzleSlotComponent* Slot : Slots)
	{
		if (Slot)
			Slot->OnSlotChanged.AddDynamic(this, &APuzzleStele::HandleSlotChanged);
	}
}
 
EPuzzleResult APuzzleStele::CheckCode()
{
	if (Slots.Num() == 0 || SecretCode.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("CheckCode: Slots ou SecretCode vide"));
		return EPuzzleResult::Incomplete;
	}

	for (UPuzzleSlotComponent* Slot : Slots)
	{
		if (!Slot || Slot->IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("CheckCode: Slot vide ou null"));
			return EPuzzleResult::Incomplete;
		}
	}

	bool bCorrect = (Slots.Num() == SecretCode.Num());
	for (int32 i = 0; i < Slots.Num(); ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("Slot[%d] = %d | SecretCode[%d] = %d"),
			i, Slots[i]->GetPlacedValue(), i, SecretCode[i]);

		if (Slots[i]->GetPlacedValue() != SecretCode[i])
		{
			bCorrect = false;
			break;
		}
	}

	if (bCorrect)
	{
		//if (SuccessSound)
			//UGameplayStatics::PlaySoundAtLocation(this, SuccessSound, GetActorLocation());
		
		if (SuccessSound)
		{
			if (SteleAudioComponent && SteleAudioComponent->IsPlaying())
				SteleAudioComponent->Stop();
 
			SteleAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
				this, SuccessSound, GetActorLocation());
		}
		
		OnCodeValidated.Broadcast();

		if (LinkedDoor)
			LinkedDoor->Open();
		else
			UE_LOG(LogTemp, Error, TEXT("CheckCode: LinkedDoor est NULL !"));

		return EPuzzleResult::Correct;
	}
	
	//if (FailSound)
		//UGameplayStatics::PlaySoundAtLocation(this, FailSound, GetActorLocation());
	if (FailSound)
	{
		if (SteleAudioComponent && SteleAudioComponent->IsPlaying())
			SteleAudioComponent->Stop();
 
		SteleAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
			this, FailSound, GetActorLocation());
	}

	OnCodeWrong.Broadcast();
	return EPuzzleResult::Wrong;
}
 
bool APuzzleStele::TrySnapDigitToNearestSlot(ANumberGrab* Digit)
{
	if (!Digit) return false;
 
	UPuzzleSlotComponent* BestSlot  = nullptr;
	float                  BestDist = FLT_MAX;
 
	for (UPuzzleSlotComponent* Slot : Slots)
	{
		if (!Slot || !Slot->IsEmpty()) continue;
 
		float Dist = FVector::Dist(Digit->GetActorLocation(), Slot->GetComponentLocation());
		if (Dist < Slot->SnapRadius && Dist < BestDist)
		{
			BestDist = Dist;
			BestSlot = Slot;
		}
	}
 
	if (BestSlot)
		return BestSlot->TryPlaceDigit(Digit);
 
	return false;
}
 
void APuzzleStele::HandleSlotChanged(UPuzzleSlotComponent* Slot)
{
	CheckCode();
}

