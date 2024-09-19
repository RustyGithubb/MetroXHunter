/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "BaseDoor.generated.h"

class ALock;

/**
 * Base class for the Doors.
 * Handles opening & closing, and keeps track of all the locks on a door.
 */
UCLASS()
class METROXHUNTER_API ABaseDoor : public ABaseInteractable
{
	GENERATED_BODY()

public:
	void BeginPlay() override;
	void Interact() override;

	UFUNCTION( BlueprintCallable, Category = "Door" )
	virtual void OnDoorHit( AActor* Player );

	UFUNCTION( BlueprintImplementableEvent, Category = "Door" )
	void SetDoorOpened( bool bShouldOpen );

	UFUNCTION( BlueprintImplementableEvent, Category = "Door" )
	void CloseDoorEditor();

	UFUNCTION( CallInEditor, Category = "Door" )
	void AddLock();

	UFUNCTION( CallInEditor, Category = "Door" )
	void RemoveAllLocks();

	UFUNCTION()
	void RemoveLock( ALock* Lock );

protected:
	UPROPERTY( EditAnywhere )
	bool bIsLocked = false;

	UPROPERTY( BlueprintReadWrite, Category = "Door" )
	bool bIsOpened = false;

	/* The minimum velocity required for the player to slam the door */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Door" )
	float MinimumVelocityRequired = 251.f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Door|Lock" )
	TArray<ALock*> LocksList;

	UPROPERTY( EditAnywhere, Category = "Door|Lock" )
	UStaticMesh* LockMesh;
};