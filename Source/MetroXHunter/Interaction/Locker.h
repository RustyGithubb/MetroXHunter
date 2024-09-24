/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "Locker.generated.h"

class UInputMappingContext;
class UInputAction;
class USceneComponent;
class ABasePickUp;

/*
 * A locker with potential pickups loot inside. To open it the Player needs to succeed a skill check.
 */
UCLASS()
class METROXHUNTER_API ALocker : public ABaseInteractable
{
	GENERATED_BODY()

public:
	ALocker();

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Locker|Loot" )
	float ItemAmount = 11;

	UPROPERTY( EditAnywhere, Category = "Locker|Loot" )
	USceneComponent* ItemSpawnPoint = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Locker|Loot" )
	TSubclassOf<ABasePickUp> ItemToSpawn = nullptr;

	/* Controllers */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Interaction|Inputs", meta = ( AllowPrivateAccess = "true" ) )
	TSoftObjectPtr<UInputAction> InteractAction = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Interaction|Inputs", meta = ( AllowPrivateAccess = "true" ) )
	TSoftObjectPtr<UInputAction> CancelInteractAction = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Interaction|Inputs", meta = ( AllowPrivateAccess = "true" ) )
	TSoftObjectPtr<UInputAction> RotateAction = nullptr;

protected:
	void Interact() override;
	void OnCancelInteraction() override;

	UFUNCTION( BlueprintImplementableEvent, Category = "Locker|SkillCheck" )
	void OnSkillCheckAttempt();

	UFUNCTION( BlueprintImplementableEvent, Category = "Locker|SkillCheck" )
	void ShowSkillCheckWidget();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Locker|SkillCheck" )
	void RemoveSkillCheckWidget();

	UFUNCTION( BlueprintCallable, Category = "Locker|SkillCheck" )
	void EndSkillCheck();

	UFUNCTION( BlueprintCallable, Category = "Locker|Loot" )
	void SpawnLootItem();

protected:
	UPROPERTY( BlueprintReadOnly, Category = "Locker|SkillCheck" )
	bool bIsSkillCheckActive = false;

private:
	void BindInputs();
	void UnBindInputs();
};
