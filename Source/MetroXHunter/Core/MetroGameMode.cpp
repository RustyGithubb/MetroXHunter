/*
 * Implemented by Corentin Paya
 * and Arthur Cathelain for the WorldLoading part.
 */

#include "MetroGameMode.h"

#include "Core/MetroGameInstance.h"
#include "SaveGame/MetroSaveGame.h"
#include "Settings/MetroSettingsSubsystem.h"

#include "Library/UtilityLibrary.h"

#include "Kismet/GameplayStatics.h"

#include "LevelInstance/LevelInstanceLevelStreaming.h"
#include "Streaming/LevelStreamingDelegates.h"

#include "AsyncLoadingScreenLibrary.h"

DEFINE_LOG_CATEGORY( LogMetroWorldLoading );

void AMetroGameMode::BeginPlay()
{
	Super::BeginPlay();

	PrepareWorldLoading();

	GameInstance = CastChecked<UMetroGameInstance>( GetGameInstance() );

	// Late Begin Play
	GetWorld()->OnWorldBeginPlay.AddUObject( this, &AMetroGameMode::LateBeginPlay );
}

void AMetroGameMode::LateBeginPlay()
{
	// Apply gameplay-related settings
	UMetroSettingsSubsystem* SettingsSubsystem = GameInstance->GetSubsystem<UMetroSettingsSubsystem>();
	SettingsSubsystem->ApplySettingsUponGameStart();
}

void AMetroGameMode::RestartFromLastCheckpoint()
{
	// Check if a save needs to be loaded before the level initialization.
	if ( GameInstance->HasSaveGame() )
	{
		GameInstance->LoadLastSave();
	}

	UGameplayStatics::OpenLevel( this, FName( *GetWorld()->GetName() ), false );
}

bool AMetroGameMode::IsWorldFullyLoaded() const
{
	return bIsWorldFullyLoaded;
}

void AMetroGameMode::OnLevelStreamingStateChanged(
	UWorld* World,
	const ULevelStreaming* StreamingLevel,
	ULevel* LoadedLevel,
	ELevelStreamingState PreviousState, ELevelStreamingState NewState
)
{
	UE_LOG(
		LogMetroWorldLoading, Verbose,
		TEXT( "Level streaming of %s state changed from %s to %s" ),
		*StreamingLevel->GetWorldAssetPackageName(),
		EnumToString( PreviousState ),
		EnumToString( NewState )
	);

	// Ensure we are loading the world
	if ( bIsWorldFullyLoaded ) return;

	// Only wait for level instances
	bool bIsLevelInstance = Cast<ULevelStreamingLevelInstance>( StreamingLevel ) != nullptr;
	if ( !bIsLevelInstance ) return;

	switch ( NewState )
	{
		case ELevelStreamingState::Loading:
		{
			LoadingLevels.Add( StreamingLevel );
			UE_LOG(
				LogMetroWorldLoading, Log,
				TEXT( "Marked level instance %s as Loading (bIsLevelInstance=%d; %d remaining)" ),
				*StreamingLevel->GetWorldAssetPackageName(),
				bIsLevelInstance,
				LoadingLevels.Num()
			);
			break;
		}
		// arkaht: This is a key call because after this, it may try to switch to MakingVisible (if the level instance should be visible).
		case ELevelStreamingState::LoadedNotVisible:
		{
			if ( PreviousState != NewState ) return;

			break;
		}
		// arkaht: This may be called after a call from LoadedNotVisible to LoadedNotVisible (yes it has the same state).
		//         For now we don't seem to need any integration about it so I'll leave it only for archive purposes.
		case ELevelStreamingState::MakingVisible:
		{
			break;
		}
		case ELevelStreamingState::LoadedVisible:
		{
			LoadingLevels.Remove( StreamingLevel );
			UE_LOG(
				LogMetroWorldLoading, Log,
				TEXT( "Fully loaded level %s (bIsLevelInstance=%d; %d remaining)" ),
				*StreamingLevel->GetWorldAssetPackageName(),
				bIsLevelInstance,
				LoadingLevels.Num()
			);

			if ( LoadingLevels.IsEmpty() )
			{
				TriggerWorldFullyLoaded();
			}
			break;
		}
	}
}

void AMetroGameMode::PrepareWorldLoading()
{
#if WITH_EDITOR
	// Workaround for no-trigger when levels are already all loaded inside the editor.
	FTimerHandle TimerHandle {};
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		[&]() {
			if ( LoadingLevels.IsEmpty() )
			{
				TriggerWorldFullyLoaded();
			}
		},
		1.0f,
		false
	);
#endif

	FLevelStreamingDelegates::OnLevelStreamingStateChanged.AddUObject( this, &AMetroGameMode::OnLevelStreamingStateChanged );
	UE_LOG( LogMetroWorldLoading, Log, TEXT( "Bound to level streaming loading" ) );
}

void AMetroGameMode::TriggerWorldFullyLoaded()
{
	verify( GameInstance != nullptr );

	// Check if a save needs to be loaded after the level initialization.
	if ( GameInstance->HasSaveGame() )
	{
		GameInstance->LoadLevel();
	}
	else
	{
		GameInstance->CreateNewSave();
	}

	UE_LOG( LogMetroWorldLoading, Log, TEXT( "Calling OnWorldFullyLoaded event" ) );

	BP_OnWorldFullyLoaded();
	OnWorldFullyLoaded.Broadcast();
	bIsWorldFullyLoaded = true;

	UAsyncLoadingScreenLibrary::StopLoadingScreen();
}
