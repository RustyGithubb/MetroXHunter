/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/GeneralInteractable.h"
#include "Locker.generated.h"

/*
 * A locker with potential Pickups loot inside. To open it the Player needs to succeed a skill check.
 */
UCLASS()
class METROXHUNTER_API ALocker : public AGeneralInteractable
{
	GENERATED_BODY()

public:
	ALocker();

public:
	UPROPERTY( EditAnywhere, Category = "Locker|Skill Check" )
	float BlendExp = 1.0f;

	UPROPERTY( EditAnywhere, Category = "Locker|Skill Check" )
	float BlendTime = 0.3f;

	UPROPERTY( EditAnywhere, Category = "Locker|Skill Check" )
	TEnumAsByte <EViewTargetBlendFunction> TargetBlendFunction = EViewTargetBlendFunction::VTBlend_EaseInOut;

protected:
	void Interact() override;
	void OnCancelInteraction();

	UFUNCTION( BlueprintImplementableEvent, Category = "Locker|Skill Check" )
	void OnSkillCheckAttempt();

	UFUNCTION( BlueprintImplementableEvent, Category = "Locker|Skill Check" )
	void ShowSkillCheckWidget();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Locker|Skill Check" )
	void RemoveSkillCheckWidget();

	UFUNCTION( BlueprintCallable, Category = "Locker|Skill Check" )
	void EndSkillCheck();

protected:
	UPROPERTY( BlueprintReadOnly, Category = "Locker|Skill Check" )
	bool bIsSkillCheckActive = false;

private:
	void BindToInputs();
	void UnbindInputs();
};
