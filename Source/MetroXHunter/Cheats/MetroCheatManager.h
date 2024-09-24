#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MetroCheatManager.generated.h"

class UMetroCheatFunction;

/**
 *
 */
UCLASS()
class METROXHUNTER_API UMetroCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	void InitCheatManager() override;

	UFUNCTION( BlueprintCallable, Category = "CheatManager" )
	void ReloadCheatFunctions();

	UFUNCTION( BlueprintImplementableEvent )
	void OnCheatFunctionRegistered( UMetroCheatFunction* CheatFunction );

	/*
	 * Find a registered cheat function matching the given subclass.
	 *
	 * @return Cheat Function instance or nullptr if not found
	 */
	UFUNCTION( BlueprintCallable, Category = "CheatManager" )
	UMetroCheatFunction* FindCheatFunctionOfClass( const TSubclassOf<UMetroCheatFunction> Class );

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FName CheatFunctionAssetsPath = TEXT( "/Game/Desgin/CheatFunctions/" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TArray<FString> CategoriesOrder {};

	UPROPERTY( BlueprintReadOnly )
	TArray<UMetroCheatFunction*> CheatFunctions {};

private:
	//  TODO: Move this function in an utility class so it's available anywhere
	void ForceLoadAssetsAtPath( FName Path );
	void InstantiateCheatFunction( const TSubclassOf<UMetroCheatFunction>& Class );

};
