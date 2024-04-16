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

	UFUNCTION( BlueprintCallable, Category="Cheat Manager" )
	void ReloadCheatFunctions();

	UFUNCTION( BlueprintImplementableEvent )
	void OnCheatFunctionRegistered( UMXHCheatFunction* CheatFunction );

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FName CheatFunctionAssetsPath = TEXT( "/Game/Cheats/Functions/" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TMap<FString, int> PriorityByCategory;

	UPROPERTY( BlueprintReadOnly )
	TArray<UMXHCheatFunction*> CheatFunctions;

private:
	//  TODO: Move this function in an utility class so it's available anywhere
	void ForceLoadAssetsAtPath( FName Path );
	void InstantiateCheatFunction( const TSubclassOf<UMXHCheatFunction>& Class );

};
