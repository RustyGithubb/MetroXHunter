/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "Elevator.generated.h"

 /**
  *
  */
UCLASS()
class METROXHUNTER_API AElevator : public ABaseInteractable
{
	GENERATED_BODY()


public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Elevator", Meta = ( MakeEditWidget = true ) )
	FVector TargetLocation;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Elevator" )
	bool bIsElevatorMoving = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Elevator", meta = ( UIMin = 1, UIMax = 5 ) )
	float ElevatorSpeed = 2.f;

protected:
	void Interact() override;

	UFUNCTION( BlueprintCallable, Category = "Elevator" )
	void EndInteraction();
};
