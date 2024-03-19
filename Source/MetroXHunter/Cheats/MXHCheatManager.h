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
	UPROPERTY( BlueprintReadOnly )
	TArray<UMXHCheatFunction*> CheatFunctions;

private:
	void _InstantiateCheatFunction( const TSubclassOf<UMXHCheatFunction>& Class );

};
