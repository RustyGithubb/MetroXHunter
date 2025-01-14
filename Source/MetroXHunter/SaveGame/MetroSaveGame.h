/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Checkpoint/BaseSavedData.h"
#include "MetroSaveGame.generated.h"

class UHealthComponent;
class UReloadComponent;
class UInventoryComponent;
class USaveLoadComponent;
struct FInstancedStruct;

/*
 * Custom Save Game that save and load the data of the objects.
 * TODO: Rename as UMetroSaveGame
 */
UCLASS()
class METROXHUNTER_API UMetroSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/* Save additional data for an actor (the structure inherits from BaseSavedData) */
	void SaveExtraProperties( const FGuid& InGuid, FInstancedStruct& InSavedData );
	/* Load additional data for an actor (the structure inherits from BaseSavedData) */
	void LoadExtraProperties( const FGuid& InGuid, FInstancedStruct& InLoadedData );

public:
	UPROPERTY( BlueprintReadOnly, SaveGame )
	FName LevelName = NAME_None;

	UPROPERTY( BlueprintReadOnly, SaveGame )
	bool bIsGameSaved = false;

	UPROPERTY( BlueprintReadOnly, SaveGame )
	TArray<FName> StreamedLevelNames {};

	UPROPERTY( SaveGame )
	TMap<FGuid, FHealthSavedData> SavedHealthComponent {};
	UPROPERTY( SaveGame )
	TMap<FGuid, FReloadSavedData> SavedReloadComponent {};
	UPROPERTY( SaveGame )
	TMap<FGuid, FInventorySavedData> SavedInventoryComponent {};

private:
	UPROPERTY( SaveGame )
	TMap<FGuid, FBaseSavedData> SavedActorData {};

	UPROPERTY( SaveGame )
	TMap<FGuid, FInteractableSavedData> SavedInteractableData {};
};
