/*
 * Implemented by Corentin Paya
 */

#include "SaveLoadComponent.h"

#include "Core/MetroGameInstance.h"
#include "SaveGame/MetroSaveGame.h"

#include <Kismet/GameplayStatics.h>

#include "Library/UtilityLibrary.h"
#include "Health/HealthComponent.h"

USaveLoadComponent::USaveLoadComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USaveLoadComponent::BeginPlay()
{
	Super::BeginPlay();

	if ( DefaultGUID.IsValid() )
	{
		ActorGUID = DefaultGUID;
	}

	GameInstance = GetWorld()->GetGameInstanceChecked<UMetroGameInstance>();

	GameInstance->OnSaveLevel.AddUniqueDynamic( this, &USaveLoadComponent::Save );
	GameInstance->OnLoadLevel.AddUniqueDynamic( this, &USaveLoadComponent::Load );
}

void USaveLoadComponent::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	UWorld* World = GetWorld();
	if ( IsValid( World ) && !World->IsGameWorld() && !IsTemplate() )
	{
		// in editor, it's fine to postpone the initialization for the next tick, 
		// until all children emitters will be initialized for sure
		World->GetTimerManager().SetTimerForNextTick(
			[this] {
				this->GenerateActorID();
			}
		);
	}
#endif
}

void USaveLoadComponent::GenerateActorID()
{
	if ( !ActorGUID.IsValid() )
	{
		DefaultGUID = DefaultGUID.NewGuid();
		ActorGUID = DefaultGUID;
	}
}

const FGuid& USaveLoadComponent::GetActorID() const
{
	return ActorGUID;
}

void USaveLoadComponent::Save()
{
	if ( !ActorGUID.IsValid() )
	{
		UUtilityLibrary::PrintWarning(
			TEXT( "SaveLoadComponent: failed to save, %s has no GUID" ),
			*GetOwner()->GetName()
		);
		return;
	}

	// Call the event before the saved to update the values on the actor (Transform).
	OnSaveActor.Broadcast();

	// Save all components in ComponentsToSave,
	// only if they implement the ISaveable interface
	for ( auto Component : ComponentsToSave )
	{
		if ( Component->GetClass()->ImplementsInterface( USaveable::StaticClass() ) )
		{
			ISaveable::Execute_OnSaveData( Component, ActorGUID, GameInstance->TemporarySaveGame );
		}
	}

	// Save extra data if needed
	if ( SavedData.IsValid() )
	{
		GameInstance->TemporarySaveGame->SaveExtraProperties( ActorGUID, SavedData );
	}

	bIsDataLoaded = false;
}

void USaveLoadComponent::Load()
{
	if ( !GameInstance->HasSaveGame() || bIsDataLoaded ) return;

	if ( !ActorGUID.IsValid() )
	{
		UUtilityLibrary::PrintWarning(
			TEXT( "SaveLoadComponent: failed to load, %s has no GUID" ),
			*GetOwner()->GetName()
		);
		return;
	}

	// Load all components in ComponentsToSave,
	// only if they implement the ISaveable interface
	for ( auto Component : ComponentsToSave )
	{
		if ( Component->GetClass()->ImplementsInterface( USaveable::StaticClass() ) )
		{
			ISaveable::Execute_OnLoadData( Component, ActorGUID, GameInstance->SaveGame );
		}
	}

	// Load extra data if needed
	if ( SavedData.IsValid() )
	{
		GameInstance->SaveGame->LoadExtraProperties( ActorGUID, SavedData );
	}

	// Call the event after load to update the actor with the new values.
	OnLoadActor.Broadcast();
	bIsDataLoaded = true;
}
