/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EQSContextProvider.generated.h"

UINTERFACE( MinimalAPI )
class UEQSContextProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class METROXHUNTER_API IEQSContextProvider
{
	GENERATED_BODY()

public:
	/*
	 * Returns the spawning location of the AI for EQS purposes.
	 */
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable )
	FVector GetEQSStartLocation() const;
	
	/*
	 * Returns the current target actor of the AI for EQS purposes.
	 */
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable )
	AActor* GetEQSTargetActor() const;
};
