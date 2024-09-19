/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "Locker.generated.h"

class USceneComponent;

/*
 * A locker with potential Pickups loot inside. To open it the Player needs to succeed a skill check.
 */
UCLASS()
class METROXHUNTER_API ALocker : public ABaseInteractable
{
	GENERATED_BODY()

public:
	ALocker();

public:
	UPROPERTY( EditAnywhere, Category = "Locker|Skill Check" )
	TEnumAsByte <EViewTargetBlendFunction> TargetBlendFunction = EViewTargetBlendFunction::VTBlend_EaseInOut;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Locker|Loot" )
	float ItemAmount = 11;

	UPROPERTY( EditAnywhere, Category = "Locker|Loot" )
	USceneComponent* ItemSpawnPoint = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Locker|Loot" )
	TSubclassOf<class ABasePickUp> ItemToSpawn = nullptr;

protected:
	void Interact() override;
	void OnCancelInteraction() override;

	UFUNCTION( BlueprintImplementableEvent, Category = "Locker|Skill Check" )
	void OnSkillCheckAttempt();

	UFUNCTION( BlueprintImplementableEvent, Category = "Locker|Skill Check" )
	void ShowSkillCheckWidget();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Locker|Skill Check" )
	void RemoveSkillCheckWidget();

	UFUNCTION( BlueprintCallable, Category = "Locker|Skill Check" )
	void EndSkillCheck();

	UFUNCTION( BlueprintCallable, Category = "Locker|Loot" )
	void SpawnLootItem();

protected:
	UPROPERTY( BlueprintReadOnly, Category = "Locker|Skill Check" )
	bool bIsSkillCheckActive = false;

private:
	void BindToInputs();
	void UnbindInputs();
};
