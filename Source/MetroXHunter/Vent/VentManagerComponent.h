/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Debug/TickDebugger.h"
#include "VentManagerComponent.generated.h"

class AVent;
class UVentData;

/*
 * Enum representing all different zones in the map that can be assigned to vents.
 */
UENUM( BlueprintType )
enum class EVentZone : uint8
{
	A		UMETA( DisplayName = "Zone A" ),
	B		UMETA( DisplayName = "Zone B" ),
	C		UMETA( DisplayName = "Zone C" ),

	MAX		UMETA( Hidden ),
};

/*
 * Component managing the vent system of the game.
 * It allows parasites to flee inside vents and to exit later in other parts of the map.
 * It is also designed to establish tension with sounds.
 * 
 * Parasites can leave the vents either by a proximity spawn or a cyclic spawn.
 * Both methods have their own timer to cooldown themselves and manage pacing.
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UVentManagerComponent : public UActorComponent, public ITickDebugger
{
	GENERATED_BODY()

public:
	UVentManagerComponent();

	virtual void BeginPlay() override;

	void TickDebug_Implementation( float DeltaTime, FString& OutDebugText ) override;

	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	void AddEnemyInZone( EVentZone Zone, int32 Amount = 1 );
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	void RemoveEnemyInZone( EVentZone Zone, int32 Amount = 1 );
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	int32 GetEnemiesCounterInZone( EVentZone Zone ) const;

	/*
	 * Starts the timer and schedule a cyclic spawn.
	 */
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	void StartCyclicSpawnTimer();
	/*
	 * Stops the timer and cancels the on-going cyclic spawn.
	 */
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	void StopCyclicSpawnTimer();
	/*
	 * Returns whenever the zone has enough enemies for scheduling a cyclic spawn.
	 * @param Zone	Zone to check
	 */
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	bool IsZoneHasEnoughEnemiesForCyclicSpawn( EVentZone Zone ) const;
	/*
	 * Returns whenever at least one of the zone meet the required conditions for a cyclic spawn.
	 */
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	bool ShouldCyclicSpawnTimerRunning() const;

	/*
	 * Resets the timer putting proximity spawning in a cooldown.
	 */
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	void ResetProximitySpawnTimer();
	/*
	 * Returns whenever the timer for the proximity spawn is depleted.
	 */
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	bool IsProximitySpawnTimerDepleted() const;

	/*
	* Finds all vents in the game and registers them.
	*/
	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	void RetrieveVents();

	UFUNCTION( BlueprintCallable, Category = "VentManager" )
	void RegisterVent( AVent* Vent );

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "VentManager" )
	UVentData* DataAsset = nullptr;
	UPROPERTY( EditAnywhere, Category = "VentManager" )
	int32 ZoneEnemies[(int32)EVentZone::MAX] {};

private:
	void CreateNextIdleSoundTimer();
	void PlayIdleSound();

	AVent* FindMostLookedVent( const TArray<AVent*>& Vents );
	AVent* TryChangeChoosenVent( TArray<AVent*> Vents, AVent* ChoosenVent );

	void OnDepleteCyclicSpawnTimer();

	UFUNCTION()
	void OnVentEndPlay( AActor* Actor, EEndPlayReason::Type Reason );

	APawn* GetPlayer();

private:
	APawn* PlayerPawn = nullptr;

	TArray<AVent*> Vents {};

	FTimerHandle IdleSoundTimerHandle {};
	FTimerHandle CyclicSpawnTimerHandle {};
	FTimerHandle ProximitySpawnTimerHandle {};
};
