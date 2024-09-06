#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TelemetryComponent.generated.h"

/*
 * Relevant game data to track and save for analysis purposes.
 */
USTRUCT( BlueprintType )
struct FTelemetryData
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Checkpoint" )
	float LastCheckpointTime = 0.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Checkpoint" )
	FString CheckpointName = "";

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Bullets" )
	int32 BulletsShotCount = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Bullets" )
	int32 BulletsHitCount = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Damage" )
	int32 DamageFromMelee = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Damage" )
	int32 DamageFromShock = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Damage" )
	int32 DamageFromExplosion = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Death" )
	int32 DeathCount = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dodge" )
	int32 DodgeOpportunitiesCount = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dodge" )
	int32 DodgeSuccessesCount = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload" )
	int32 ReloadsTotalCount = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload" )
	int32 ReloadsFailedCount = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload" )
	int32 ReloadsActiveCount = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Reload" )
	int32 ReloadsPerfectCount = 0;
};

/*
 * Component tracking game data for game design statistics analysis.
 * Data is written into a file to the 'Saved/Playtest/' directory using CSV format.
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UTelemetryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTelemetryComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type Reason ) override;

	/*
	 * Saves all tracked game data into a file.
	 */
	void WriteDataToFile();
	/*
	 * Clears all tracked game data.
	 */
	void ClearData();

public:
	/*
	 * All game data to track, save and analyse.
	 */
	UPROPERTY( BlueprintReadWrite, Category = "Telemetry" )
	FTelemetryData Data;

private:
	/*
	 * Appends a text to the buffer.
	 */
	void AppendToBuffer( const FString& Text );
	/*
	 * Clears the buffer.
	 */
	void ClearBuffer();

	/*
	 * Constructs a file path for the given playtest session.
	 */
	FString GetFilePath( const FDateTime& DateTime, const FString& CheckpointName ) const;

private:
	/*
	 * String data eventually written to a file.
	 */
	FString Buffer;
};
