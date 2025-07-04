/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/CheatManagerDefines.h"
#include "ConvarLibrary.generated.h"

/*
 * Library giving easy access to the custom console variables of the project.
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

	UFUNCTION( BlueprintPure, Category = "ConvarLibrary", meta = ( DisplayName = "GetAIPlayerTokenOverrideConvarValue" ) )
	static int32 GetAIPlayerTokenOverride();

	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsAINoQTEConvarEnabled();

	UFUNCTION( BlueprintCallable, Category = "ConvarLibrary" )
	static void SetTickDebuggerDefaultNameConvarValue( const FString& Value );
	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static FString GetTickDebuggerDefaultNameConvarValue();

	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsGunDebugEnabled();

	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsGunInfiniteAmmoEnabled();

	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsPlayerInvincibleEnabled();

	UFUNCTION( BlueprintPure, Category = "ConvarLibrary" )
	static bool IsPlayerSkipCinematicEnabled();
};
