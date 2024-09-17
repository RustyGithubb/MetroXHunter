#include "Telemetry/TelemetryComponent.h"

#include "MXHUtilityLibrary.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetTextLibrary.h"

UTelemetryComponent::UTelemetryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTelemetryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Retrieve owner creation time if ever the component is added at runtime
	Data.LastCheckpointTime = GetOwner()->GetGameTimeSinceCreation();
}

void UTelemetryComponent::EndPlay( const EEndPlayReason::Type Reason )
{
	switch ( Reason )
	{
		case EEndPlayReason::LevelTransition:
		case EEndPlayReason::EndPlayInEditor:
		case EEndPlayReason::Quit:
			WriteDataToFile();
			break;
	}
}

void UTelemetryComponent::WriteDataToFile()
{
	// Compute play time and updates checkpoint time
	float NewCheckpointTime = GetOwner()->GetGameTimeSinceCreation();
	float PlayTime = NewCheckpointTime - Data.LastCheckpointTime;
	Data.LastCheckpointTime = NewCheckpointTime;

	// Append user name
	FString UserName = UKismetSystemLibrary::GetPlatformUserName();
	// Anti-dysphorie original code
	UserName = UserName.Replace( TEXT( "leonard" ), TEXT( "miya" ), ESearchCase::CaseSensitive );

	// Compute date time
	FDateTime DateTimeNow = UKismetMathLibrary::Now();
	FString DateTime = UKismetTextLibrary::AsDateTime_DateTime( DateTimeNow ).ToString();
	DateTime = DateTime.Replace( TEXT( "," ), TEXT( "" ), ESearchCase::IgnoreCase );

	ClearBuffer();

	// Write session data
	AppendToBuffer( UserName );
	AppendToBuffer( DateTime );
	AppendToBuffer( Data.CheckpointName );
	AppendToBuffer( FString::SanitizeFloat( PlayTime ) );

	// Write bullets stats
	int32 ShotAccuracy = (float)Data.BulletsHitCount / Data.BulletsShotCount;
	AppendToBuffer( FString::FromInt( Data.BulletsShotCount ) );
	AppendToBuffer( FString::FromInt( Data.BulletsHitCount ) );
	AppendToBuffer( FString::FromInt( ShotAccuracy * 100 ) );

	// Write damage stats
	AppendToBuffer( FString::FromInt( Data.DamageFromMelee ) );
	AppendToBuffer( FString::FromInt( Data.DamageFromShock ) );
	AppendToBuffer( FString::FromInt( Data.DamageFromExplosion ) );

	// Write death count
	AppendToBuffer( FString::FromInt( Data.DeathCount ) );

	// Write dodge stats
	int32 DodgeAccuracy = (float)Data.DodgeSuccessesCount / Data.DodgeOpportunitiesCount;
	AppendToBuffer( FString::FromInt( Data.DodgeOpportunitiesCount ) );
	AppendToBuffer( FString::FromInt( Data.DodgeSuccessesCount ) );
	AppendToBuffer( FString::FromInt( DodgeAccuracy * 100 ) );

	// Write reload stats
	int32 ReloadAccuracy = (float)( Data.ReloadsActiveCount + Data.ReloadsPerfectCount ) / Data.ReloadsTotalCount;
	int32 ReloadsPassiveCount = Data.ReloadsTotalCount - Data.ReloadsFailedCount - Data.ReloadsActiveCount - Data.ReloadsPerfectCount;
	AppendToBuffer( FString::FromInt( Data.ReloadsFailedCount ) );
	AppendToBuffer( FString::FromInt( Data.ReloadsActiveCount ) );
	AppendToBuffer( FString::FromInt( Data.ReloadsPerfectCount ) );
	AppendToBuffer( FString::FromInt( ReloadsPassiveCount ) );
	AppendToBuffer( FString::FromInt( Data.ReloadsTotalCount ) );
	AppendToBuffer( FString::FromInt( ReloadAccuracy * 100 ) );

	//  Write buffer to file
	UMXHUtilityLibrary::SaveFileAsString( Buffer, GetFilePath( DateTimeNow, Data.CheckpointName ) );

	ClearData();
}

void UTelemetryComponent::ClearData()
{
	Data = FTelemetryData {};
}

void UTelemetryComponent::AppendToBuffer( const FString& Text )
{
	Buffer += Text + ",";
}

void UTelemetryComponent::ClearBuffer()
{
	Buffer.Reset();
}

FString UTelemetryComponent::GetFilePath( const FDateTime& DateTime, const FString& CheckpointName ) const
{
	return FString::Printf(
		TEXT( "%s/Playtest/%s_Data_%s_Checkpoint%s.csv" ),
		*FPaths::ProjectSavedDir(),
		UMXHUtilityLibrary::IsWithinEditor() ? TEXT( "PIE" ) : TEXT( "Build" ),
		*DateTime.ToFormattedString( TEXT( "%d/%b_%H:%M:%S" ) ),
		*CheckpointName
	);
}