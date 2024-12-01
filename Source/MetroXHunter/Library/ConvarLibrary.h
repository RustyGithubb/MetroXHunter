/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ConvarLibrary.generated.h"

/*
 * 
 */
UCLASS()
class METROXHUNTER_API UConvarLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/*
	 * Returns whenever the AI Debug convar is enabled.
	 * @return Convar is enabled
	 */
	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsAIDebugConvarEnabled();
	/*
	 * Returns whenever the AI Ignore Player convar is enabled.
	 * @return Convar is enabled
	 */
	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsAIIgnorePlayerConvarEnabled();

	UFUNCTION( BlueprintCallable, Category = "ConvarLibrary" )
	static void SetTickDebuggerDefaultNameConvarValue( const FString& Value );
	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static FString GetTickDebuggerDefaultNameConvarValue();

	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsGunDebugEnabled();
};
