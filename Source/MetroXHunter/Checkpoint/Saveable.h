/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Checkpoint/SaveLoadComponent.h"
#include "Checkpoint/BaseSavedData.h"
#include "SaveGame/MetroSaveGame.h"

#include "Saveable.generated.h"

UINTERFACE( MinimalAPI )
class USaveable : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class METROXHUNTER_API ISaveable
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "EnemySpawnerInterface" )
	void OnLoadData( const FGuid& ActorID, UMetroSaveGame* SaveGame );
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "EnemySpawnerInterface" )
	void OnSaveData( const FGuid& ActorID, UMetroSaveGame* SaveGame);
};
