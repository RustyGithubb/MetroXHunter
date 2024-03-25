#include "MXHCheatManager.h"

#include "MXHCheatFunction.h"

#include <AssetRegistry/AssetRegistryModule.h>

void UMXHCheatManager::InitCheatManager()
{
	ReloadCheatFunctions();

	Super::InitCheatManager();
}

void UMXHCheatManager::ReloadCheatFunctions()
{
	//  Clear current array
	CheatFunctions.Empty();

	//  NOTE: Force load the assets before iterating over the UClasses, otherwise, 
	//  they are not findable
	ForceLoadAssetsAtPath( CheatFunctionAssetsPath );

	//  Iterate over all UClass to find our subclasses
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		UClass* Class = *It;

		//  Filter out non-subclasses
		if ( !Class->IsChildOf<UMXHCheatFunction>() ) continue;

		//  Filter out the base class
		if ( Class->HasAnyClassFlags( CLASS_Abstract ) ) continue;

		//  Filter out any skeleton blueprints
		//if ( It->GetName().RemoveFromStart( "SKEL" ) ) continue;

		//  Filter out any non-blueprints classes
		if ( !Class->GetName().RemoveFromStart( "BP_" ) ) continue;

		//  Instantiate cheat function
		InstantiateCheatFunction( Class );

		UE_LOG( LogTemp, Log, TEXT( "New Cheat Function: %s" ),
			*It->GetName() );
	}

	UE_LOG( LogTemp, Log, TEXT( "Total of %d Cheat Functions" ),
		CheatFunctions.Num() );
}

void UMXHCheatManager::ForceLoadAssetsAtPath( FName Path )
{
	//  Load asset registry module
	FName ModuleName( "AssetRegistry" );
	auto& AssetRegistryModule = 
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>( ModuleName );
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	//  Scan path
	TArray<FString> Paths;
	Paths.Add( Path.ToString() );
	AssetRegistry.ScanPathsSynchronous( Paths );

	//  Get assets in path
	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByPath( Path, Assets, true );

	//  Force loading all assets
	for ( const auto& AssetData : Assets )
	{
		auto Asset = AssetData.GetAsset();
		UE_LOG( LogTemp, Log, TEXT( "Force Load Asset: %s" ), 
			*Asset->GetPathName() );
	}
}

void UMXHCheatManager::InstantiateCheatFunction(
	const TSubclassOf<UMXHCheatFunction>& Class
)
{
	auto CheatFunction = NewObject<UMXHCheatFunction>( this, Class );
	CheatFunction->Init( this );
	CheatFunctions.Add( CheatFunction );
}
