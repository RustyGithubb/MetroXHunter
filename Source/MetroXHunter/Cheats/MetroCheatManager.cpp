#include "MetroCheatManager.h"

#include "MetroCheatFunction.h"
#include "UtilityLibrary.h"

#include <AssetRegistry/AssetRegistryModule.h>

void UMetroCheatManager::InitCheatManager()
{
	ReloadCheatFunctions();

	Super::InitCheatManager();
}

void UMetroCheatManager::ReloadCheatFunctions()
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
		if ( !Class->IsChildOf<UMetroCheatFunction>() ) continue;

		//  Filter out the base class
		if ( Class->HasAnyClassFlags( CLASS_Abstract ) ) continue;

		//  Filter out any skeleton blueprints
		//if ( It->GetName().RemoveFromStart( "SKEL" ) ) continue;

		//  Filter out any non-blueprints classes
		if ( !Class->GetName().RemoveFromStart( "BP_" ) ) continue;

		//  Instantiate cheat function
		InstantiateCheatFunction( Class );

		UUtilityLibrary::LogMessage(
			TEXT( "New Cheat Function: %s" ),
			*It->GetName()
		);
	}

	UUtilityLibrary::LogMessage(
		TEXT( "Total of %d Cheat Functions" ),
		CheatFunctions.Num()
	);

	//  Sort functions first by category and second by name
	CheatFunctions.Sort(
		[&]( const UMetroCheatFunction& a, const UMetroCheatFunction& b ) {
			const FString CategoryA = a.Category.ToString();
			const FString CategoryB = b.Category.ToString();

			const int32 CategoryOrderA = CategoriesOrder.Find( CategoryA );
			const int32 CategoryOrderB = CategoriesOrder.Find( CategoryB );

			if ( CategoryOrderA == CategoryOrderB
			  || CategoryOrderA == INDEX_NONE || CategoryOrderB == INDEX_NONE )
			{
				return a.Name.ToString() < b.Name.ToString();
			}

			return CategoryOrderA < CategoryOrderB;
		}
	);
}

UMetroCheatFunction* UMetroCheatManager::FindCheatFunctionOfClass( const TSubclassOf<UMetroCheatFunction> Class )
{
	for ( UMetroCheatFunction* CheatFunction : CheatFunctions )
	{
		if ( CheatFunction->GetClass() != Class ) continue;
		return CheatFunction;
	}

	return nullptr;
}

void UMetroCheatManager::ForceLoadAssetsAtPath( FName Path )
{
	//  Load asset registry module
	auto& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>( 
		TEXT( "AssetRegistry" )
	);
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	//  Scan path
	TArray<FString> Paths {};
	Paths.Add( Path.ToString() );
	AssetRegistry.ScanPathsSynchronous( Paths );

	//  Get assets in path
	TArray<FAssetData> Assets {};
	AssetRegistry.GetAssetsByPath( Path, Assets, true );

	//  Force loading all assets
	for ( const auto& AssetData : Assets )
	{
		auto Asset = AssetData.GetAsset();
		UUtilityLibrary::LogMessage(
			TEXT( "Force Load Asset: %s" ),
			*Asset->GetPathName()
		);
	}
}

void UMetroCheatManager::InstantiateCheatFunction(
	const TSubclassOf<UMetroCheatFunction>& Class
)
{
	auto CheatFunction = NewObject<UMetroCheatFunction>( this, Class );
	CheatFunction->Init( this );
	CheatFunctions.Add( CheatFunction );

	//  Warn of un-registered category
	if ( !CategoriesOrder.Contains( CheatFunction->Category.ToString() ) )
	{
		UUtilityLibrary::PrintError(
			TEXT( "Cheat Function '%s' using un-registered category '%s', please update your CheatManager!" ),
			*CheatFunction->Name.ToString(), *CheatFunction->Category.ToString()
		);
	}

	OnCheatFunctionRegistered( CheatFunction );
}
