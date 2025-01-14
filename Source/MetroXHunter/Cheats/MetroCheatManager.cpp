/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "MetroCheatManager.h"

#include "MetroCheatFunction.h"
#include "Library/UtilityLibrary.h"

#include <AssetRegistry/AssetRegistryModule.h>

void UMetroCheatManager::InitCheatManager()
{
	ReloadCheatFunctions();

	Super::InitCheatManager();
}

void UMetroCheatManager::ReloadCheatFunctions()
{
	// Clear current array
	CheatFunctions.Empty();

	// NOTE: Force load the assets before iterating over the UClasses, otherwise, 
	// they are not findable
	ForceLoadAssetsAtPath( CheatFunctionAssetsPath );

	// Iterate over all UClass to find our subclasses
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		UClass* Class = *It;

		// Filter out non-subclasses
		if ( !Class->IsChildOf<UMetroCheatFunction>() ) continue;

		// Filter out the base class
		if ( Class->HasAnyClassFlags( CLASS_Abstract ) ) continue;

		// Filter out any non-blueprints classes
		if ( !Class->GetName().RemoveFromStart( "BP_" ) ) continue;

		// Instantiate cheat function
		InstantiateCheatFunction( Class );
	}

	UUtilityLibrary::LogMessage(
		TEXT( "Total of %d Cheat Functions" ),
		CheatFunctions.Num()
	);

	// Sort functions first by category and second by name
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
	for ( auto CheatFunction : CheatFunctions )
	{
		if ( CheatFunction->GetClass() != Class ) continue;
		return CheatFunction;
	}

	return nullptr;
}

void UMetroCheatManager::Tick( float DeltaTime )
{
	if ( LastFrameTicked == GFrameCounter ) return;

	for ( auto CheatFunction : CheatFunctions )
	{
		CheatFunction->OnTick( DeltaTime );
	}

	LastFrameTicked = GFrameCounter;
}

TStatId UMetroCheatManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( UMetroCheatManager, STATGROUP_Tickables );
}

void UMetroCheatManager::ForceLoadAssetsAtPath( FName Path )
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

	// Warn of un-registered category
	if ( !CategoriesOrder.Contains( CheatFunction->Category.ToString() ) )
	{
		UUtilityLibrary::PrintWarning(
			TEXT( "Cheat Function '%s' (%s) using un-registered category '%s', please update your CheatManager!" ),
			*CheatFunction->Name.ToString(), *CheatFunction->GetName(),
			*CheatFunction->Category.ToString()
		);
	}

	// Warn about development mistakes
	for ( const auto OtherCheatFunction : CheatFunctions )
	{
		if ( OtherCheatFunction == CheatFunction ) continue;
		if ( !OtherCheatFunction->Category.EqualTo( CheatFunction->Category ) ) continue;

		// Compare names
		if ( OtherCheatFunction->Name.EqualTo( CheatFunction->Name ) )
		{
			UUtilityLibrary::PrintWarning(
				TEXT( "Cheat Function '%s' (%s) using the same name than '%s' (%s), please update one of them!" ),
				*CheatFunction->Name.ToString(), *CheatFunction->GetName(),
				*OtherCheatFunction->Name.ToString(), *OtherCheatFunction->GetName()
			);
			break;
		}
	}

	OnCheatFunctionRegistered( CheatFunction );

	UUtilityLibrary::LogMessage( TEXT( "New Cheat Function: %s" ), *Class->GetName() );
}