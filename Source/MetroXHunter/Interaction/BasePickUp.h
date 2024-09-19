/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"

#include "BasePickUp.generated.h"

class UInventoryComponent;
enum class EPickupType : uint8;

/*
 * Base class for the PickUps
 */
UCLASS()
class METROXHUNTER_API ABasePickUp : public ABaseInteractable
{
	GENERATED_BODY()

public:
	ABasePickUp();
	virtual void BeginPlay() override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int Amount = 0;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	EPickupType PickupType;

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	UTexture2D* Sprite;

protected:
	void Interact() override;

	void OnInnerCircleOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult
	) override;

	void OnInteractableTargeted() override;

private:
	UInventoryComponent* PlayerInventory = nullptr;
};
