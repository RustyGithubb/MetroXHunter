#include "Cheats/MXHCheatManager.h"

#include "CheatFunction.h"

void UMXHCheatManager::InitCheatManager()
{
	ReloadCheatFunctions();

	Super::InitCheatManager();
}

void UMXHCheatManager::ReloadCheatFunctions()
{
	//  Clear current array
	CheatFunctions.Empty();

	//  Iterate over all UClass to find our subclasses
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		UClass* Class = *It;

		//  Filter out non-subclasses
		if ( !Class->IsChildOf<UCheatFunction>() ) continue;

		//  Filter out the base class
		if ( Class->HasAnyClassFlags( CLASS_Abstract ) ) continue;

		//  Filter out any skeleton blueprints
		//if ( It->GetName().RemoveFromStart( "SKEL" ) ) continue;

		//  Filter out any non-blueprints classes
		if ( !Class->GetName().RemoveFromStart( "BP_" ) ) continue;

		//  Instantiate cheat function
		_InstantiateCheatFunction( Class );

		UE_LOG( LogTemp, Log, TEXT( "New Cheat Function: %s" ),
			*It->GetName() );
	}

	UE_LOG( LogTemp, Log, TEXT( "Total of %d Cheat Functions" ),
		CheatFunctions.Num() );
}

void UMXHCheatManager::_InstantiateCheatFunction(
	const TSubclassOf<UCheatFunction>& Class
)
{
	auto CheatFunction = NewObject<UCheatFunction>( this, Class );
	CheatFunction->Init( this );
	CheatFunctions.Add( CheatFunction );
}
