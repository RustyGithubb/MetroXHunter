/*
 * Implemented by Corentin Paya and Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInputHandler.generated.h"

UINTERFACE( MinimalAPI )
class UPlayerInputHandler : public UInterface
{
	GENERATED_BODY()
};

/*
 * Interface of the Player Controller to add and remove input mapping context
 */
class METROXHUNTER_API IPlayerInputHandler
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "PlayerController|Inputs" )
	void SetInputMappingContext( UInputMappingContext* MappingContext );

	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "PlayerController|Inputs" )
	void ResetInputMappingContext();
};
