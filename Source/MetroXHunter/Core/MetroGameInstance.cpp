/*
 * Implemented by Corentin Paya
 */

#include "MetroGameInstance.h"

#include "Core/MetroGameMode.h"
#include "SaveGame/MetroSaveGame.h"

#include "Kismet/GameplayStatics.h"

#include "Library/UtilityLibrary.h"

DEFINE_LOG_CATEGORY( LogMetroSaveSystem );

void UMetroGameInstance::Init()
{
	Super::Init();
}

void UMetroGameInstance::CreateNewSave()
{
	SaveGame = Cast<UMetroSaveGame>(
		UGameplayStatics::CreateSaveGameObject(
			UMetroSaveGame::StaticClass()
		)
	);

	DeleteOldSaves();
}

void UMetroGameInstance::DeleteOldSaves()
{
	// Get the path to the save directory
	FString SaveDirectory = FPaths::ProjectSavedDir() + TEXT( "SaveGames/" );

	// Get the file manager instance
	IFileManager& FileManager = IFileManager::Get();

	// Check if the directory exists
	if ( FileManager.DirectoryExists( *SaveDirectory ) )
	{
		// Find all files in the save directory
		TArray<FString> Files;
		FileManager.FindFiles( Files, *SaveDirectory, TEXT( "*.*" ) ); // Change the wildcard if necessary

		// Loop through and delete each file
		for ( const FString& File : Files )
		{
			FString FullFilePath = SaveDirectory + File;
			if ( FileManager.Delete( *FullFilePath ) )
			{
				UE_LOG( LogTemp, Log, TEXT( "Deleted file: %s" ), *FullFilePath );
			}
			else
			{
				UE_LOG( LogTemp, Warning, TEXT( "Failed to delete file: %s" ), *FullFilePath );
			}
		}
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "Save directory does not exist: %s" ), *SaveDirectory );
	}
}

void UMetroGameInstance::LoadLastSave()
{
	if ( UGameplayStatics::DoesSaveGameExist( SaveName, UserIndex ) )
	{
		SaveGame = Cast<UMetroSaveGame>( UGameplayStatics::LoadGameFromSlot( SaveName, UserIndex ) );
	}
}

// TODO: Move this inside UtilityLibrary
static FName GetLevelName( const TSoftObjectPtr<UWorld> Level )
{
	// We return the exact same name that is used by Unreal to stream levels.
	// See GameplayStatics.cpp:914
	return FName( *FPackageName::ObjectPathToPackageName( Level.ToString() ) );
}

void UMetroGameInstance::SaveLevel()
{
	if ( !SaveGame )
	{
		CreateNewSave();
	}

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Saving game started" ) );

	OnSaveLevel.Broadcast();

	SaveGame->LevelName = GetLevelName( GetWorld() );
	SaveGame->bIsGameSaved = true;
	SaveGame->StreamedLevelNames = CurrentStreamedLevelNames.Array();

	for ( const FName& LevelName : SaveGame->StreamedLevelNames )
	{
		UE_LOG( LogMetroSaveSystem, Log, TEXT( "Saved level streaming: %s" ), *LevelName.ToString() );
	}

	UGameplayStatics::SaveGameToSlot( SaveGame, SaveName, UserIndex );

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Saving game completed" ) );
}

void UMetroGameInstance::LoadLevel()
{
	verify( IsValid( SaveGame ) );

	const FName WorldLevelName = GetLevelName( GetWorld() );
	if ( !ensureAlwaysMsgf( SaveGame->LevelName == WorldLevelName,
		TEXT( "GameInstance: Tried to load a save game of level %s inside level %s" ),
		*SaveGame->LevelName.ToString(),
		*WorldLevelName.ToString() ) 
	) return;

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Loading game started" ) );

	CurrentStreamedLevelNames.Empty();

	for ( const FName& LevelName : SaveGame->StreamedLevelNames )
	{
		UE_LOG( LogMetroSaveSystem, Log, TEXT( "Loading level streaming: %s" ), *LevelName.ToString() );

		// Register as a current streamed level to avoid missing some levels when re-saving.
		// TODO: Add it in LoadLevelStreaming and use this function to control level streaming.
		CurrentStreamedLevelNames.Add( LevelName );

		// Load level streaming (implemented in blueprint to avoid managing a FLatentInfo struct)
		LoadLevelStreaming( LevelName );
	}

	OnLoadLevel.Broadcast();

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Loading game completed" ) );
}

void UMetroGameInstance::RegisterStreamedLevel( const TSoftObjectPtr<UWorld> Level )
{
	const FName LevelName = GetLevelName( Level );
	CurrentStreamedLevelNames.Add( LevelName );

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Registered level streaming: %s" ), *LevelName.ToString() );
}

void UMetroGameInstance::UnRegisterStreamedLevel( const TSoftObjectPtr<UWorld> Level )
{
	const FName LevelName = GetLevelName( Level );
	CurrentStreamedLevelNames.Remove( LevelName );

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Un-registered level streaming: %s" ), *LevelName.ToString() );
}

void UMetroGameInstance::LoadLevelStreamingByObject( const TSoftObjectPtr<UWorld> Level )
{
	const FName LevelName = GetLevelName( Level );
	LoadLevelStreaming( LevelName );
}

bool UMetroGameInstance::HasSaveGame() const
{
	if ( !IsValid( SaveGame ) ) return false;
	return SaveGame->bIsGameSaved && SaveGame->LevelName == GetLevelName( GetWorld() );
}
