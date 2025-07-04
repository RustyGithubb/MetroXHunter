/*
 * Implemented by Corentin Paya
 */

#include "MetroGameInstance.h"

#include "Core/MetroGameMode.h"
#include "SaveGame/MetroSaveGame.h"
#include "Kismet/GameplayStatics.h"

#include "AudioDeviceHandle.h"
#include "AudioDeviceManager.h"
#include "AudioMixer.h"
#include "Engine/Engine.h"

#include "Library/UtilityLibrary.h"

constexpr auto GLOBAL_SAVE_NAME = TEXT( "GlobalSaved" );

DEFINE_LOG_CATEGORY( LogMetroSaveSystem );

void UMetroGameInstance::Init()
{
	Super::Init();

	// Get base FAudioDevice
	FAudioDevice* AudioDevice = GetWorld()->GetAudioDeviceRaw();
	// Cast to FMixerDevice* ( need AudioMixerDevice.h & AudioMixer )
	Audio::FMixerDevice* MixerDevice = static_cast<Audio::FMixerDevice*>( AudioDevice );
	// Get platform interface
	Audio::IAudioMixerPlatformInterface* PlatformInterface = MixerDevice->GetAudioMixerPlatform();
	check( PlatformInterface );
}

void UMetroGameInstance::CreateNewSave()
{
	UGameplayStatics::DeleteGameInSlot( GLOBAL_SAVE_NAME, UserIndex );

	SaveGame = Cast<UMetroSaveGame>(
		UGameplayStatics::CreateSaveGameObject(
			UMetroSaveGame::StaticClass()
		)
	);

	TemporarySaveGame = Cast<UMetroSaveGame>(
	UGameplayStatics::CreateSaveGameObject(
		UMetroSaveGame::StaticClass()
	)
	);

	CurrentStreamedLevelNames.Empty();
	CurrentUnloadStreamedLevelNames.Empty();
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
	if ( UGameplayStatics::DoesSaveGameExist( GLOBAL_SAVE_NAME, UserIndex ) )
	{
		SaveGame = Cast<UMetroSaveGame>( UGameplayStatics::LoadGameFromSlot( GLOBAL_SAVE_NAME, UserIndex ) );
		TemporarySaveGame = SaveGame;
	}
}

void UMetroGameInstance::SaveLevel()
{
	if ( !SaveGame )
	{
		CreateNewSave();
	}

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Saving game started" ) );

	OnSaveLevel.Broadcast();
	SaveGame = TemporarySaveGame;

	SaveGame->LevelName = UUtilityLibrary::GetLevelName( GetWorld() );
	SaveGame->bIsGameSaved = true;
	SaveGame->StreamedLevelNames = CurrentStreamedLevelNames.Array();
	SaveGame->UnloadStreamedLevelNames = CurrentUnloadStreamedLevelNames.Array();

	for ( const FName& LevelName : SaveGame->StreamedLevelNames )
	{
		UE_LOG( LogMetroSaveSystem, Log, TEXT( "Saved loaded level streaming: %s" ), *LevelName.ToString() );
	}

	for ( const FName& LevelName : SaveGame->UnloadStreamedLevelNames )
	{
		UE_LOG( LogMetroSaveSystem, Log, TEXT( "Saved unloaded level streaming: %s" ), *LevelName.ToString() );
	}

	UGameplayStatics::SaveGameToSlot( SaveGame, GLOBAL_SAVE_NAME, UserIndex );

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Saving game completed" ) );
}

void UMetroGameInstance::LoadLevel()
{
	verify( IsValid( SaveGame ) );

	const FName WorldLevelName = UUtilityLibrary::GetLevelName( GetWorld() );
	if ( !ensureAlwaysMsgf( SaveGame->LevelName == WorldLevelName,
		TEXT( "GameInstance: Tried to load a save game of level %s inside level %s" ),
		*SaveGame->LevelName.ToString(),
		*WorldLevelName.ToString() ) 
	) return;

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Loading game started" ) );

	CurrentStreamedLevelNames.Empty();
	CurrentUnloadStreamedLevelNames.Empty();

	for ( const FName& LevelName : SaveGame->StreamedLevelNames )
	{
		UE_LOG( LogMetroSaveSystem, Log, TEXT( "Loading level streaming: %s" ), *LevelName.ToString() );

		// Register as a current streamed level to avoid missing some levels when re-saving.
		CurrentStreamedLevelNames.Add( LevelName );

		// Load level streaming (implemented in blueprint to avoid managing a FLatentInfo struct)
		LoadLevelStreaming( LevelName );
	}

	for ( const FName& LevelName : SaveGame->UnloadStreamedLevelNames )
	{
		UE_LOG( LogMetroSaveSystem, Log, TEXT( "Unloading level streaming: %s" ), *LevelName.ToString() );

		// Register as a current streamed level to avoid missing some levels when re-saving.
		CurrentUnloadStreamedLevelNames.Add( LevelName );

		// Unload level streaming (implemented in blueprint to avoid managing a FLatentInfo struct)
		UnloadLevelStreaming( LevelName );
	}

	OnLoadLevel.Broadcast();
	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Loading game completed" ) );
}

void UMetroGameInstance::RegisterStreamedLevel( const TSoftObjectPtr<UWorld> Level )
{
	const FName LevelName = UUtilityLibrary::GetLevelName( Level );
	CurrentStreamedLevelNames.Add( LevelName );

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Registered loaded level streaming: %s" ), *LevelName.ToString() );
}

void UMetroGameInstance::UnRegisterStreamedLevel( const TSoftObjectPtr<UWorld> Level )
{
	const FName LevelName = UUtilityLibrary::GetLevelName( Level );
	CurrentUnloadStreamedLevelNames.Add( LevelName );

	UE_LOG( LogMetroSaveSystem, Log, TEXT( "Registered unloaded level streaming: %s" ), *LevelName.ToString() );
}

void UMetroGameInstance::LoadLevelStreamingByObject( const TSoftObjectPtr<UWorld> Level )
{
	const FName LevelName = UUtilityLibrary::GetLevelName( Level );
	LoadLevelStreaming( LevelName );
}

bool UMetroGameInstance::HasSaveGame() const
{
	if ( !IsValid( SaveGame ) ) return false;
	return SaveGame->bIsGameSaved && SaveGame->LevelName == UUtilityLibrary::GetLevelName( GetWorld() );
}

void UMetroGameInstance::SetAmmoMultiplier(float Multiplier)
{
	AmmoMultiplier = Multiplier;
}

TArray<FAudioDeviceInfoBP> UMetroGameInstance::GetAvailableOutputDevices() const
{
    TArray<FAudioDeviceInfoBP> OutDevices;

    // Try to get the raw FAudioDevice* from the AudioDeviceManager
    FAudioDevice* AudioDevice = nullptr;
    if ( GEngine )
    {
        if ( FAudioDeviceManager* DeviceManager = GEngine->GetAudioDeviceManager() )
        {
            // WorldContext
            if ( GetWorld() )
            {
                AudioDevice = DeviceManager->GetAudioDeviceFromWorldContext( GetWorld() );
            }
            // Fallback to the main device
            if ( !AudioDevice )
            {
                AudioDevice = DeviceManager->GetMainAudioDeviceRaw();
            }
        }
    }

    if ( !AudioDevice )
    {
        UE_LOG( LogTemp, Warning, TEXT( "No FAudioDevice available" ) );
        return OutDevices;
    }

    // Cast to FMixerDevice to use the Audio Mixer API
    auto* MixerDevice = static_cast<Audio::FMixerDevice*>( AudioDevice );
    if ( !MixerDevice )
    {
        UE_LOG( LogTemp, Warning, TEXT( "AudioMixer is not active" ) );
        return OutDevices;
    }

    // Get the platform interface for enumeration
    auto* Platform = MixerDevice->GetAudioMixerPlatform();
    if ( !Platform )
    {
        UE_LOG( LogTemp, Warning, TEXT( "AudioMixerPlatformInterface not available" ) );
        return OutDevices;
    }

    // Enumerate output devices
    uint32 DeviceCount = 0;
    Platform->GetNumOutputDevices( DeviceCount );
    OutDevices.Reserve( DeviceCount );

    for ( uint32 Index = 0; Index < DeviceCount; ++Index )
    {
        Audio::FAudioPlatformDeviceInfo Info;
        if ( Platform->GetOutputDeviceInfo( Index, Info ) )
        {
            FAudioDeviceInfoBP BPInfo;
            BPInfo.Name = Info.Name;
            BPInfo.DeviceId = Info.DeviceId;
            OutDevices.Add( BPInfo );
        }
    }

    // If no devices found, add a "System Default" fallback so that Blueprints never receive an empty array
    if ( OutDevices.Num() == 0 )
    {
        FAudioDeviceInfoBP DefaultBP;
        DefaultBP.Name = TEXT( "System Default" );
        DefaultBP.DeviceId = TEXT( "" );  // empty ID means OS default device
        OutDevices.Add( DefaultBP );
        UE_LOG( LogTemp, Warning, TEXT( "No audio devices detected, added 'System Default' fallback" ) );
    }

    return OutDevices;
}

void UMetroGameInstance::SetOutputDevice( const FString& DeviceId, bool& bSuccess )
{
    bSuccess = false;

    // Try to get the raw FAudioDevice* from the AudioDeviceManager
    FAudioDevice* AudioDevice = nullptr;
    if ( GEngine )
    {
        if ( FAudioDeviceManager* DeviceManager = GEngine->GetAudioDeviceManager() )
        {
            if ( GetWorld() )
            {
                AudioDevice = DeviceManager->GetAudioDeviceFromWorldContext( GetWorld() );
            }
            if ( !AudioDevice )
            {
                AudioDevice = DeviceManager->GetMainAudioDeviceRaw();
            }
        }
    }

    if ( !AudioDevice )
    {
        UE_LOG( LogTemp, Warning, TEXT( "No FAudioDevice available; skipping device switch" ) );
        return;
    }

    // Cast to FMixerDevice to use the Audio Mixer API
    auto* MixerDevice = static_cast<Audio::FMixerDevice*>( AudioDevice );
    if ( !MixerDevice )
    {
        UE_LOG( LogTemp, Warning, TEXT( "AudioMixer is not active; skipping device switch" ) );
        return;
    }

    // Get the platform interface for switch
    auto* Platform = MixerDevice->GetAudioMixerPlatform();
    if ( !Platform )
    {
        UE_LOG( LogTemp, Warning, TEXT( "AudioMixerPlatformInterface not available; skipping device switch" ) );
        return;
    }

    // Prevent against zero-device
    uint32 DeviceCount = 0;
    Platform->GetNumOutputDevices( DeviceCount );
    if ( DeviceCount == 0 )
    {
        UE_LOG( LogTemp, Warning, TEXT( "No audio devices available; skipping device switch" ) );
        return;
    }

    // Attempt to switch the audio device
    bSuccess = Platform->MoveAudioStreamToNewAudioDevice( DeviceId );
    if ( bSuccess )
    {
        Platform->StartAudioStream();
        UE_LOG( LogTemp, Log, TEXT( "Audio routed to DeviceId=%s" ), *DeviceId );
    }
    else
    {
        UE_LOG( LogTemp, Warning, TEXT( "Failed to route audio to DeviceId=%s" ), *DeviceId );
    }
}
