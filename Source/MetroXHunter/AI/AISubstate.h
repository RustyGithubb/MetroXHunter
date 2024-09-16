/* 
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AISubstate.generated.h"

/**
 * 
 */
UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UAISubstate : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "AISubstate" )
	void OnSubstateEntered();
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "AISubstate" )
	void OnSubstateExited();
};