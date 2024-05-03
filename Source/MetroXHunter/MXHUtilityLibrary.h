#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MXHUtilityLibrary.generated.h"

/**
 * 
 */
UCLASS()
class METROXHUNTER_API UMXHUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
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
	/**
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
	/**
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

	/**
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
	/**
	 * Prints to screen with a custom color and time and logs a message to the output log.
	 *
	 * @param Text	Format text
	 * @param Color Displayed text color on screen
	 * @param Time  Duration of displayed text on screen
	 * @param Args	Format arguments
	 */
	template<typename TFormat, typename... TArgs>
	static void PrintMessage( const TFormat& Text, const FColor& Color = FColor::Blue, const float Time = 5.0f, TArgs... Args )
	{
		const FString FormattedText = FString::Printf( Text, Args... );
		UE_LOG( LogTemp, Log, TEXT( "%s" ), *FormattedText );
		GEngine->AddOnScreenDebugMessage( INDEX_NONE, Time, Color, FormattedText );
	}
	/**
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
	/**
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

	UFUNCTION( BlueprintCallable, BlueprintPure )
	static bool IsCVarAIDebugEnabled();
};
