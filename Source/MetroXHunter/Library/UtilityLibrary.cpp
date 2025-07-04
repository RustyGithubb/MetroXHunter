#include "Library/UtilityLibrary.h"

#include "AssetRegistry/AssetRegistryModule.h"

#include "Engine/Console.h"

float UUtilityLibrary::RandomInRange( const FFloatRange& Range )
{
	const float LowerValue = Range.GetLowerBoundValue();
	const float UpperValue = Range.GetUpperBoundValue();

	if ( LowerValue == UpperValue ) return LowerValue;

	return FMath::RandRange( LowerValue, UpperValue );
}

bool UUtilityLibrary::SaveFileAsString( const FString& Data, const FString& Path )
{
	return FFileHelper::SaveStringToFile( Data, *Path );
}

bool UUtilityLibrary::LoadFileAsString( const FString& Path, FString& Data )
{
	return FFileHelper::LoadFileToString( Data, *Path );
}

FString UUtilityLibrary::GetSoftObjectPathAssetName( const FSoftObjectPath& Path )
{
	return Path.GetAssetName();
}

void UUtilityLibrary::ForceLoadAssetsAtPath( FName Path )
{
	// Load asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		TEXT( "AssetRegistry" )
	);
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Scan path
	TArray<FString> Paths {};
	Paths.Add( Path.ToString() );
	AssetRegistry.ScanPathsSynchronous( Paths );

	// Get assets in path
	TArray<FAssetData> Assets {};
	AssetRegistry.GetAssetsByPath( Path, Assets, true );

	// Force loading all assets
	for ( const FAssetData& AssetData : Assets )
	{
		// NOTE: GetAsset ensure the asset is loaded even with const
		const UObject* Asset = AssetData.GetAsset();
		if ( Asset == nullptr ) continue;

		LogMessage(
			TEXT( "Force Load Asset: %s" ),
			*Asset->GetPathName()
		);
	}
}

FName UUtilityLibrary::GetLevelName( const TSoftObjectPtr<UWorld> Level )
{
	// We return the exact same name that is used by Unreal to stream levels.
	// See GameplayStatics.cpp:914
	const FString PackageName = FPackageName::ObjectPathToPackageName( Level.ToString() );

	// Except we remove the prefix when in editor for more accurate comparisons.
	// It is probably not enough if the UEDPIE's number is different from 0 but it is working for now.
	const FString NicerName = PackageName.Replace( TEXT( "UEDPIE_0_" ), TEXT( "" ) );
	return FName( *NicerName );
}

bool UUtilityLibrary::IsWithinEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

FString UUtilityLibrary::GetProjectVersion()
{
	return FString::Printf( TEXT( "%s - %s" ), TEXT( __DATE__ ), TEXT( __TIME__ ) );
}

static const FName NAME_Typing = FName( TEXT( "Typing" ) );
static const FName NAME_Open = FName( TEXT( "Open" ) );

static void SetConsoleState( UObject* WorldContextObject, FName NewConsoleState )
{
	if ( !IsValid( WorldContextObject ) ) return;

	UWorld* World = WorldContextObject->GetWorld();
	if ( !IsValid( World ) ) return;

	UGameViewportClient* GameViewport = World->GetGameViewport();
	if ( !IsValid( GameViewport ) ) return;

	// Console might be null, like in shipping builds or on consoles.
	UConsole* ViewportConsole = WorldContextObject->GetWorld()->GetGameViewport()->ViewportConsole;
	if ( IsValid( ViewportConsole ) )
	{
		ViewportConsole->FakeGotoState( NewConsoleState );
	}
}

void UUtilityLibrary::SetConsoleOpened( UObject* WorldContextObject, bool bIsOpened, bool bIsFull )
{
	if ( bIsOpened )
	{
		SetConsoleState( WorldContextObject, bIsFull ? NAME_Open : NAME_Typing );
	}
	else
	{
		SetConsoleState( WorldContextObject, NAME_None );
	}
}
