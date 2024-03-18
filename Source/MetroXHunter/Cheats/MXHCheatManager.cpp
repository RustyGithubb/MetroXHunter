#include "Cheats/MXHCheatManager.h"

void UMXHCheatManager::InitCheatManager()
{
	_FindCheatFunctionsClasses();

	Super::InitCheatManager();
}

void UMXHCheatManager::_FindCheatFunctionsClasses()
{
	//  Clear current array
	CheatFunctionClasses.Empty();

	//  Iterate over all UClass to find our subclasses
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		//  Filter out non-subclasses
		if ( !It->IsChildOf<UCheatFunction>() ) continue;

		//  Filter out the base class
		if ( It->HasAnyClassFlags( CLASS_Abstract ) ) continue;
		
		//  Filter out any skeleton blueprints
		//if ( It->GetName().RemoveFromStart( "SKEL" ) ) continue;

		//  Filter out any non-blueprints classes
		if ( !It->GetName().RemoveFromStart( "BP_" ) ) continue;

		//  Add to array
		CheatFunctionClasses.Add( *It );

		//  Instantiate cheat function
		auto CheatFunction = NewObject<UCheatFunction>( this, *It );
		CheatFunction->Init( this );
		CheatFunctions.Add( CheatFunction );

		UE_LOG( LogTemp, Log, TEXT( "New Cheat Function: %s" ), 
			*It->GetName() );
	}

	UE_LOG( LogTemp, Log, TEXT( "Total of %d Cheat Functions" ), 
		CheatFunctionClasses.Num() );
}
