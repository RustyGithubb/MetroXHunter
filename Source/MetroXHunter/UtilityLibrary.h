#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UtilityLibrary.generated.h"

/**
 * 
 */
UCLASS()
class METROXHUNTER_API UUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/*
	 * Log a message to the output log.
	 * 
	 * @param Text	Format text
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void LogMessage( const TFormat& Text, TArgs... Args )
	{
		const FString FormattedText = FString::Printf( Text, Args... );
		UE_LOG( LogTemp, Log, TEXT( "%s" ), *FormattedText );
	}
	/*
	 * Log a warning to the output log.
	 *
	 * @param Text	Format text
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void LogWarning( const TFormat& Text, TArgs... Args )
	{
		const FString FormattedText = FString::Printf( Text, Args... );
		UE_LOG( LogTemp, Warning, TEXT( "%s" ), *FormattedText );
	}
	/*
	 * Log an error to the output log.
	 *
	 * @param Text	Format text
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void LogError( const TFormat& Text, TArgs... Args )
	{
		const FString FormattedText = FString::Printf( Text, Args... );
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *FormattedText );
	}

	/*
	 * Prints to screen and logs a message to the output log.
	 *
	 * @param Text Text
	 */
	template<typename TText>
	static void PrintMessage( const TText& Text )
	{
		PrintMessage( Text, TEXT( "" ) );
	}
	/*
	 * Prints to screen and logs a message to the output log.
	 *
	 * @param Text	Format text
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void PrintMessage( const TFormat& Text, TArgs... Args )
	{
		PrintMessage( Text, FColor::Blue, 5.0f, Args... );
	}
	/*
	 * Prints to screen with a custom color and time and logs a message to the output log.
	 *
	 * @param Text	Format text
	 * @param Color Displayed text color on screen
	 * @param Time  Duration of displayed text on screen
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void PrintMessage( 
		const TFormat& Text,
		const FColor& Color = FColor::Blue,
		const float Time = 5.0f,
		TArgs... Args
	)
	{
		const FString FormattedText = FString::Printf( Text, Args... );
		UE_LOG( LogTemp, Log, TEXT( "%s" ), *FormattedText );
		GEngine->AddOnScreenDebugMessage( INDEX_NONE, Time, Color, FormattedText );
	}
	/*
	 * Prints to screen and logs a warning to the output log.
	 *
	 * @param Text Text
	 */
	template<typename TFormat>
	static void PrintWarning( const TFormat& Text )
	{
		PrintWarning( Text, TEXT( "" ) );
	}
	/*
	 * Prints to screen and logs a warning to the output log.
	 *
	 * @param Text	Format text
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void PrintWarning( const TFormat& Text, TArgs... Args )
	{
		const FString FormattedText = FString::Printf( Text, Args... );
		UE_LOG( LogTemp, Warning, TEXT( "%s" ), *FormattedText );
		GEngine->AddOnScreenDebugMessage( INDEX_NONE, 5.0f, FColor::Yellow, FormattedText );
	}
	/*
	 * Prints to screen and logs an error to the output log.
	 *
	 * @param Text Text
	 */
	template<typename TFormat>
	static void PrintError( const TFormat& Text )
	{
		PrintError( Text, TEXT( "" ) );
	}
	/*
	 * Prints to screen and logs an error to the output log.
	 *
	 * @param Text	Format text
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void PrintError( const TFormat& Text, TArgs... Args )
	{
		const FString FormattedText = FString::Printf( Text, Args... );
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *FormattedText );
		GEngine->AddOnScreenDebugMessage( INDEX_NONE, 10.0f, FColor::Red, FormattedText );
	}

	/*
	 * Get components by tag of a given actor. 
	 * Internally use AActor::GetComponentsByTag, but return the correct array type.
	 * 
	 * @param Actor Actor to get the components from
	 * @param Tag   Tag to search for
	 * @return Array of components in correct type
	 */
	template<typename ComponentType>
	static TArray<ComponentType*> GetComponentsOfActorByTag( AActor* Actor, const FName& Tag )
	{
		auto Components = Actor->GetComponentsByTag( ComponentType::StaticClass(), Tag );

		TArray<ComponentType*> OutComponents {};
		OutComponents.Reserve( Components.Num() );
		for ( auto Component : Components )
		{
			OutComponents.Add( CastChecked<ComponentType>( Component ) );
		}

		return MoveTemp( OutComponents );
	}

	template<typename ArrayElementType>
	static ArrayElementType& PickRandomElement( TArray<ArrayElementType>& Array )
	{
		return Array[FMath::RandRange( 0, Array.Num() - 1 )];
	}

	/*
	 * Saves a string into a file at specified path. 
	 * Path is absolute.
	 * 
	 * @param Data	Serialized data to save
	 * @param Path	File path to save to
	 * @return Whenever the operation was successful
	 */
	UFUNCTION( BlueprintCallable, Category="MXHUtilityLibrary|Filesystem" )
	static bool SaveFileAsString( const FString& Data, const FString& Path );
	/*
	 * Loads a file at specified path and fills the given string with its content.
	 * Path is absolute.
	 *
	 * @param Path File path to load from
	 * @param Data String to fill with the file's content 
	 * @return Whenever the operation was successful
	*/
	UFUNCTION( BlueprintCallable, Category="MXHUtilityLibrary|Filesystem" )
	static bool LoadFileAsString( const FString& Path, FString& Data );

	/*
	 * Returns whenever the game's code has been built including the editor.
	 * 
	 * @return Whenever the editor is included
	 */
	UFUNCTION( BlueprintPure, Category="MXHUtilityLibrary" )
	static bool IsWithinEditor();

	/*
	 * Returns whenever the CVar 'MXH.AI.Debug' is enabled.
	 * 
	 * @return Whenever the CVar is enabled
	 */
	UFUNCTION( BlueprintPure, Category="MXHUtilityLibrary|CVars" )
	static bool IsCVarAIDebugEnabled();

	UFUNCTION( BlueprintPure, Category="MXHUtilityLibrary|CVars" )
	static FString GetProjectVersion();
};
