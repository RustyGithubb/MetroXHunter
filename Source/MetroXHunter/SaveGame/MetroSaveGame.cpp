/*
 * Implemented by Corentin Paya
 */

#include "MetroSaveGame.h"

#include "InstancedStruct.h"
#include "Checkpoint/Saveable.h"

#include "Health/HealthComponent.h"
#include "Reload/ReloadComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Checkpoint/SaveLoadComponent.h"

void UMetroSaveGame::SaveExtraProperties( const FGuid& InGuid, FInstancedStruct& InSavedData )
{
	// Retrieve the instantiated struct and store it in the corresponding map.
	if ( const FInteractableSavedData* InteractableData = InSavedData.GetPtr<FInteractableSavedData>() )
	{
		SavedInteractableData.Add( InGuid, *InteractableData );
	}
	else if ( const FBaseSavedData* BaseData = InSavedData.GetPtr<FBaseSavedData>() )
	{
		SavedActorData.Add( InGuid, *BaseData );
	}
}

void UMetroSaveGame::LoadExtraProperties( const FGuid& InGuid, FInstancedStruct& InLoadedData )
{
	// Retrieve the instancied struct, and load all the data corresponding to the GUID
	if ( auto InteractableData = InLoadedData.GetMutablePtr<FInteractableSavedData>() )
	{
		if ( !SavedInteractableData.Contains( InGuid ) ) return;

		InteractableData->LastLocation = SavedInteractableData[InGuid].LastLocation;
		InteractableData->bShouldSpawn = SavedInteractableData[InGuid].bShouldSpawn;
		InteractableData->bIsConsummed = SavedInteractableData[InGuid].bIsConsummed;

	}
	else if ( auto BaseData = InLoadedData.GetMutablePtr<FBaseSavedData>() )
	{
		if ( !SavedActorData.Contains( InGuid ) ) return;

		BaseData->LastLocation = SavedActorData[InGuid].LastLocation;
		BaseData->bShouldSpawn = SavedActorData[InGuid].bShouldSpawn;
	}
}
