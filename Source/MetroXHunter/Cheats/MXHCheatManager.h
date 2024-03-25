#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MXHCheatManager.generated.h"

class UMXHCheatFunction;

/**
 * 
 */
UCLASS()
class METROXHUNTER_API UMXHCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:
	void InitCheatManager() override;

	void ReloadCheatFunctions();

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FName CheatFunctionAssetsPath = TEXT( "/Game/Cheats/Functions/" );

	UPROPERTY( BlueprintReadOnly )
	TArray<UMXHCheatFunction*> CheatFunctions;

private:
	//  TODO: Move this function in an utility class so it's available anywhere
	void _ForceLoadAssetsAtPath( FName Path );
	void _InstantiateCheatFunction( const TSubclassOf<UMXHCheatFunction>& Class );

};
