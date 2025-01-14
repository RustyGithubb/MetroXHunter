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
	/*
	 * Set the given input mapping context to the current one.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "PlayerController|Inputs" )
	void SetInputMappingContext( UInputMappingContext* MappingContext );

	/*
	 * Reset the input mapping context to the default one.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "PlayerController|Inputs" )
	void ResetInputMappingContext();

	/*
	 * Reset the input mapping context to the default one only if the given
	 * mapping context is the current one.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "PlayerController|Inputs" )
	void RevertInputMappingContext( UInputMappingContext* MappingContext );
};
