/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "EnemySpawnerScriptedEvent.h"

#include "AI/AITargetComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

#include "Library/UtilityLibrary.h"

void UEnemySpawnerScriptedEvent::Tick( float DeltaTime )
{
	if ( LastTickedFrame == GFrameCounter ) return;
	LastTickedFrame = GFrameNumber;

	// Increase counter over time
	CurrentSpawnTime += DeltaTime;

	// Stop running when no spawner is remaining
	if ( ScheduledSpawners.IsEmpty() )
	{
		bIsRunning = false;
		return;
	}

	// Check for current scheduled spawner time
	const uint32 LastScheduledIndex = ScheduledSpawners.Num() - 1;
	const FScheduledEnemySpawner& CurrentScheduledSpawner = ScheduledSpawners[LastScheduledIndex];
	if ( CurrentSpawnTime < CurrentScheduledSpawner.Time ) return;

	RunSpawner( CurrentScheduledSpawner.SpawnerIndex, CurrentScheduledSpawner.SpawnIndex );
	ScheduledSpawners.RemoveAt( LastScheduledIndex );
}

bool UEnemySpawnerScriptedEvent::IsTickable() const
{
	return bIsRunning && CheckSpawnCondition();
}

TStatId UEnemySpawnerScriptedEvent::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT( UEnemySpawnerScriptedEvent, STATGROUP_Tickables );
}

void UEnemySpawnerScriptedEvent::OnRunScriptedEvent_Implementation()
{
	bIsRunning = true;
	CurrentSpawnTime = 0.0f;
	
	float NextSpawnerDelay = 0.0f;
	float NextSpawnDelay = 0.0f;

	for ( int32 SpawnerIndex = 0; SpawnerIndex < Spawners.Num(); SpawnerIndex++ )
	{
		const FEnemySpawner& Spawner = Spawners[SpawnerIndex];
		if ( !IsValid( Spawner.Actor ) ) continue;

		// Check for spawner interface
		if ( !Spawner.Actor->Implements<UEnemySpawnerInterface>() )
		{
			UUtilityLibrary::PrintWarning(
				TEXT(
					"EnemySpawnerScriptedEvent: %s doesn't implement interface EnemySpawnerInterface"
					", please use a valid spawner"
				),
				*Spawner.Actor->GetName()
			);
			continue;
		}

		// Reset spawn delay for current spawner
		NextSpawnDelay = 0.0f;

		for ( int32 SpawnIndex = 0; SpawnIndex < Spawner.SpawnInfos.Num(); SpawnIndex++ )
		{
			// Schedule spawn depending on some delays
			FScheduledEnemySpawner ScheduledSpawner {};
			ScheduledSpawner.SpawnerIndex = SpawnerIndex;
			ScheduledSpawner.SpawnIndex = SpawnIndex;
			ScheduledSpawner.Time = NextSpawnerDelay + NextSpawnDelay;
			ScheduledSpawners.Add( ScheduledSpawner );

			// Update spawn delays
			NextSpawnDelay += UUtilityLibrary::RandomInRange( Spawner.SpawnTimeRange );
			if ( bRunSpawnersOneAfterAnother )
			{
				NextSpawnerDelay += NextSpawnDelay;
			}
		}

		// Delay next spawner
		NextSpawnerDelay += UUtilityLibrary::RandomInRange( SpawnerTimeRange );
	}

	ScheduledSpawners.Sort(
		[]( const FScheduledEnemySpawner& A, const FScheduledEnemySpawner& B )
		{
			// We are sorting in the descending order to avoid removing from the beginning
			// of the array (resulting in re-hashing of the entire array)
			return A.Time > B.Time;
		}
	);
}

bool UEnemySpawnerScriptedEvent::CheckSpawnCondition() const
{
	if ( MaxAttackersCount > 0 )
	{
		const ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter( this, 0 );
		if ( auto AITargetComponent = PlayerCharacter->FindComponentByClass<UAITargetComponent>() )
		{
			return AITargetComponent->GetAttackersCount() < MaxAttackersCount;
		}
	}

	return true;
}

void UEnemySpawnerScriptedEvent::RunSpawner( int32 SpawnerIndex, int32 SpawnIndex )
{
	const FEnemySpawner& Spawner = Spawners[SpawnerIndex];
	if ( !IsValid( Spawner.Actor ) ) return;

	IEnemySpawnerInterface::Execute_SpawnEnemy( Spawner.Actor, Spawner.SpawnInfos[SpawnIndex] );
}
