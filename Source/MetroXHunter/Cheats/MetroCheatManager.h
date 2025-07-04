#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MetroCheatManager.generated.h"

class UMetroCheatFunction;

/**
 *
 */
UCLASS()
class METROXHUNTER_API UMetroCheatManager : public UCheatManager, public FTickableGameObject
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
	UFUNCTION( BlueprintCallable, Category = "CheatManager", meta = ( DeterminesOutputType = "Class" ) )
	UMetroCheatFunction* FindCheatFunctionOfClass( const TSubclassOf<UMetroCheatFunction> Class );

	void Tick( float DeltaTime ) override;
	TStatId GetStatId() const override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FName CheatFunctionAssetsPath = TEXT( "/Game/Desgin/CheatFunctions/" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TArray<FString> CategoriesOrder {};

	UPROPERTY( BlueprintReadOnly )
	TArray<UMetroCheatFunction*> CheatFunctions {};

private:
	void InstantiateCheatFunction( const TSubclassOf<UMetroCheatFunction>& Class );

private:
	/*
	 * Keeps track of the last frame ticked to avoid multiple tick call per frame
	 */
	uint32 LastFrameTicked = INDEX_NONE;
};
