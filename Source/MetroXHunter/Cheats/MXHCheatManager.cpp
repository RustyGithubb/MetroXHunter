#include "MXHCheatManager.h"

#include "MXHCheatFunction.h"
#include "MXHUtilityLibrary.h"

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

		UMXHUtilityLibrary::LogMessage(
			TEXT( "New Cheat Function: %s" ),
			*It->GetName() 
		);
	}

	UMXHUtilityLibrary::LogMessage(
		TEXT( "Total of %d Cheat Functions" ),
		CheatFunctions.Num()
	);

	//  Sort functions first by category and second by name
	CheatFunctions.Sort(
		[&]( const UMXHCheatFunction& a, const UMXHCheatFunction& b ) {
			const FString CategoryA = a.Category.ToString();
			const FString CategoryB = b.Category.ToString();

			if ( CategoryA == CategoryB
			  || !PriorityByCategory.Contains( CategoryA ) || !PriorityByCategory.Contains( CategoryB ) )
			{
				return a.Name.ToString() < b.Name.ToString();
			}

			return PriorityByCategory[CategoryA] < PriorityByCategory[CategoryB];
		}
	);
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
		UMXHUtilityLibrary::LogMessage(
			TEXT( "Force Load Asset: %s" ),
			*Asset->GetPathName() 
		);
	}
}

void UMXHCheatManager::InstantiateCheatFunction(
	const TSubclassOf<UMXHCheatFunction>& Class
)
{
	auto CheatFunction = NewObject<UMXHCheatFunction>( this, Class );
	CheatFunction->Init( this );
	CheatFunctions.Add( CheatFunction );

	//  Warn of un-registered category
	if ( !PriorityByCategory.Contains( CheatFunction->Category.ToString() ) )
	{
		UMXHUtilityLibrary::PrintError(
			TEXT( "Cheat Function '%s' using un-registered category '%s', please update your CheatManager!" ),
			*CheatFunction->Name.ToString(), *CheatFunction->Category.ToString()
		);
	}

	OnCheatFunctionRegistered( CheatFunction );
}
