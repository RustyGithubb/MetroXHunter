/*  
 * Implemented by Corentin Paya 
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/GeneralInteractable.h"
#include "PickUp.generated.h"

/**
 * 
 */
UCLASS()
class METROXHUNTER_API APickUp : public AGeneralInteractable
{
	GENERATED_BODY()
	
protected:
	UFUNCTION( BlueprintImplementableEvent, Category = "Pick Up|UI" )
	void EditSprite();

	void Interact() override;

	virtual void OnInnerCircleOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult
	);
};
