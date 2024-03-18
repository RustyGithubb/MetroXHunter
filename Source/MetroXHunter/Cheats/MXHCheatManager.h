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
	UPROPERTY( BlueprintReadOnly )
	TArray<TSubclassOf<UCheatFunction>> CheatFunctionClasses;
	UPROPERTY( BlueprintReadOnly )
	TArray<UCheatFunction*> CheatFunctions;

private:
	void _FindCheatFunctionsClasses();

};
