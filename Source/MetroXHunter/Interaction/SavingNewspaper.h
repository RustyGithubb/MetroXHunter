/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Interaction/BaseInteractable.h"
#include "SavingNewspaper.generated.h"

 /*
  * Newspaper Interactable to Save the current state of the game
  */
UCLASS()
class METROXHUNTER_API ASavingNewspaper : public ABaseInteractable
{
	GENERATED_BODY()

protected:
	void Interact() override;
};
