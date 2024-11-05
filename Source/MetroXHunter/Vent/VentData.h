/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Engine/DataAsset.h"
#include "VentData.generated.h"

class AParasite;

/**
 * 
 */
UCLASS()
class METROXHUNTER_API UVentData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	/*
	 * Enemy class to spawn.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn" )
	TSubclassOf<AParasite> EnemyClass {};
	/*
	 * Maximum amount of enemies to spawn at once in a vent.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn" )
	int32 MaxEnemiesToSpawn = 3;
	/*
	 * Range to pick a random time for delaying each enemy spawn.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn" )
	FFloatRange SpawnTimeRange { 1.5f, 3.0f };
	/*
	 * Range to pick a random time in which spawning enemies from vents is not allowed.
	 * This timer is applied each time an enemy enters the vent or when enemies are leaving it.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn|Proximity" )
	FFloatRange ProximitySpawnTimeRange { 8.0f, 16.0f };

	/*
	 * Minimum amount of enemies required in a zone to start a cyclic spawn.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn|Cyclic" )
	int32 MinEnemiesInZoneToStartCyclicSpawn = 3;
	/*
	 * Range to pick a random time in which the cyclic spawn will happen.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn|Cyclic" )
	FFloatRange CyclicSpawnTimeRange { 8.0f, 16.0f };
	/*
	 * Range of allowed distance to filter vents that are too far or too close from the player.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn|Cyclic" )
	FFloatRange CyclicSpawnDistanceRange { 200.0f, 3000.0f };
	/*
	 * Field of view of the player that helps to determine the most-looked vent by filtering
	 * all vents that are not within it.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn|Cyclic", meta = ( Units = "Degrees", ClampMin = 0, ClampMax = 180 ) )
	float CyclicSpawnMostLookedVentFOV = 60.0f;
	/*
	 * Chance percentage of rerolling the choosen vent if it was the most-looked one.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Spawn|Cyclic", meta = ( Units = "Percent", ClampMin = 0, ClampMax = 100 ) )
	int32 CyclicSpawnRerollMostLookedVentChance = 75;

	/*
	 * Range to pick a random time for idle sound cooldown.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds|Idle", meta = ( Units = "Seconds" ) )
	FFloatRange IdleSoundTimeRange { 2.0f, 6.0f };
	/*
	 * List to pick a random sound from when playing idle sound.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds|Idle" )
	TArray<USoundBase*> IdleSounds {};
	/*
	 * Volume multiplier for idle sounds.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds|Idle" )
	float IdleSoundVolumeMultiplier = 0.3f;
	/*
	 * Range of allowed distance to filter vents that are too far or too close from the player.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds|Idle" )
	FFloatRange IdleSoundDistanceRange { 0.0f, 3000.0f };

	/*
	 * Sound played when a parasite enters a vent.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds|Enter" )
	USoundBase* EnterSound = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds|Idle" )
	USoundAttenuation* SoundAttenuation = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds|Idle" )
	USoundConcurrency* SoundConcurrency = nullptr;
};
