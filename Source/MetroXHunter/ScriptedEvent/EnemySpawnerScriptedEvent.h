/*
* Implemented by Arthur Cathelain (arkaht)
*/

#pragma once

#include "CoreMinimal.h"
#include "ScriptedEventManager.h"
#include "EnemySpawnerScriptedEvent.generated.h"

class UZeroEnemyData;
class UParasiteData;

/*
 * Enum of all spawnable enemy types.
 * Designed for the EnemySpawnerScriptedEvent.
 */
UENUM( BlueprintType )
enum EEnemyType : uint8
{
	ZeroEnemy,
	Parasite,
};

/*
 * Structure representing all informations needed for a single enemy spawn.
 * Designed for the EnemySpawnerScriptedEvent.
 */
USTRUCT( BlueprintType )
struct FEnemySpawnerSpawnInfo
{
	GENERATED_BODY()

	/*
	 * Whether the enemy should automatically target the player upon spawn.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	bool bAutoAttackPlayer = true;

	/*
	 * Type of enemy to spawn.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TEnumAsByte<EEnemyType> EnemyType = EEnemyType::ZeroEnemy;
	
	/*
	 * Data Asset to apply when spawning a ZeroEnemy.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( EditCondition = "EnemyType == EEnemyType::ZeroEnemy", EditConditionHides ) )
	UZeroEnemyData* ZeroDataAsset = nullptr;

	/*
	* Data Asset to apply when spawning a Parasite.
	*/
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( EditCondition = "EnemyType == EEnemyType::Parasite", EditConditionHides ) )
	UParasiteData* ParasiteDataAsset = nullptr;
};

UINTERFACE( Blueprintable )
class UEnemySpawnerInterface : public UInterface
{
	GENERATED_BODY()
};

/*
* Interface designed to abstractly spawn one player's enemy.
* Designed for the EnemySpawnerScriptedEvent.
* This should be implemented on an actor.
*/
class METROXHUNTER_API IEnemySpawnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "EnemySpawnerInterface" )
	AActor* SpawnEnemy( const FEnemySpawnerSpawnInfo& SpawnInfo );
};

/*
 * Structure representing a single enemy spawner.
 * Designed for the EnemySpawnerScriptedEvent.
 */
USTRUCT( BlueprintType )
struct FEnemySpawner
{
	GENERATED_BODY()

	/*
	 * Spawner actor implementing the UEnemySpawnerInterface interface.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	AActor* Actor = nullptr;

	/*
	 * Array gathering information for each individual spawn.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TArray<FEnemySpawnerSpawnInfo> SpawnInfos {};

	/*
	 * Time range between each spawn.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FFloatRange SpawnTimeRange { 1.5f, 3.0f };
};

struct FScheduledEnemySpawner
{
	uint16 SpawnerIndex = 0;
	uint16 SpawnIndex = 0;

	float Time = 0.0f;
};

/*
 * Scripted Event responsible for spawning enemies with multiple spawners
 * each one having multiple spawn infos.
 */
UCLASS( BlueprintType, Blueprintable, meta = ( DisplayName = "Enemy Spawner" ) )
class METROXHUNTER_API UEnemySpawnerScriptedEvent : public UScriptedEvent, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void Tick( float DeltaTime ) override;
	virtual bool IsTickable() const override;

	virtual TStatId GetStatId() const override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	TArray<FEnemySpawner> Spawners {};

	/*
	 * Time range between each spawner.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FFloatRange SpawnerTimeRange { 1.5f, 3.0f };

	/*
	 * If set to true, the spawners will start execute only after the previous spawner finished.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	bool bRunSpawnersOneAfterAnother = false;

	/*
	 * If set above 0, the maximum number of attackers that, if reached, pause spawner's execution.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int32 MaxAttackersCount = 0;

protected:
	void OnRunScriptedEvent_Implementation() override;

private:
	bool CheckSpawnCondition() const;
	void RunSpawner( int32 SpawnerIndex, int32 SpawnIndex );

private:
	TArray<FScheduledEnemySpawner> ScheduledSpawners {};
	float CurrentSpawnTime = 0.0f;

	uint32 LastTickedFrame = INDEX_NONE;
	bool bIsRunning = false;
};