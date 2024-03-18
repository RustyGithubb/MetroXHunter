#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"

#include "CheatFunction.h"

#include "MXHCheatManager.generated.h"

/**
 * 
 */
UCLASS()
class METROXHUNTER_API UMXHCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:
	void InitCheatManager() override;


public:
	//  TODO: Remove if not necessary
	UPROPERTY( BlueprintReadOnly )
	TArray<TSubclassOf<UCheatFunction>> CheatFunctionClasses;
	UPROPERTY( BlueprintReadOnly )
	TArray<UCheatFunction*> CheatFunctions;

private:
	//  TODO: Rename the function if CheatFunctionsClasses gets removed
	void _FindCheatFunctionsClasses();

};
