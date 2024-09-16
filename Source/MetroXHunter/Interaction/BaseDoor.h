/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "BaseDoor.generated.h"

class ALock;

 /**
  *
  */
UCLASS()
class METROXHUNTER_API ABaseDoor : public ABaseInteractable
{
	GENERATED_BODY()

public:
	void Interact() override;

	UFUNCTION(BlueprintCallable)
	virtual void OnDoorHit( AActor* Player);

	UFUNCTION(BlueprintImplementableEvent)
	void SetDoorOpened( bool bShouldOpen );

	UFUNCTION(CallInEditor)
	void AddLock();

protected:
	bool bIsLocked = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsOpened = false;

	/* The minimum velocity required for the player to slam the door */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinimumVelocityRequired = 251.f;

private:
	TArray<ALock*> LocksList;
};
