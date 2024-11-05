/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "UObject/Interface.h"
#include "TickDebugger.generated.h"

UINTERFACE( MinimalAPI )
class UTickDebugger : public UInterface
{
	GENERATED_BODY()
};

/*
 * Interface to tick an object only for debug purposes.
 */
class METROXHUNTER_API ITickDebugger
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "TickDebugger" )
	void TickDebug( float DeltaTime, FString& OutDebugText );
};
