#include "Library/UtilityLibrary.h"

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
	FString Value = "";
	GConfig->GetString(
		TEXT( "/Script/EngineSettings.GeneralProjectSettings" ),
		TEXT( "ProjectVersion" ),
		Value,
		GGameIni
	);

	return Value;
}
