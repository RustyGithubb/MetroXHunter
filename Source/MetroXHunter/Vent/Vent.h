/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScriptedEvent/EnemySpawnerScriptedEvent.h"
#include "Vent/VentManagerComponent.h"
#include "Vent.generated.h"

class AParasite;
class UVentData;
class UPrimitiveComponent;

/*
 * Actor representing a vent that parasites can flee into for the vent system of our game.
 */
UCLASS()
class AVent : public AActor, public IEnemySpawnerInterface
{
	GENERATED_BODY()

public:
	AVent();

	void BeginPlay() override;

	// Begin IEnemySpawnerInterface interface
	AActor* SpawnEnemy_Implementation( const FEnemySpawnerSpawnInfo& SpawnInfo ) override;
	// End IEnemySpawnerInterface interface

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Vent" )
	void StartEnterVent( AActor* Actor );
	UFUNCTION( BlueprintCallable, Category = "Vent" )
	void EnterVent( AActor* Actor );

	UFUNCTION( BlueprintCallable, Category = "Vent" )
	void TrySpawnEnemies();

	UFUNCTION( BlueprintCallable, Category = "Vent" )
	void PlayEnterSound();
	UFUNCTION( BlueprintCallable, Category = "Vent" )
	void PlayIdleSound();
	UFUNCTION( BlueprintCallable, Category = "Vent" )
	void PlayScreamSound();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Vent" )
	USceneComponent* GetSpawnPointComponent() const;
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Vent" )
	USceneComponent* GetEntranceComponent() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEnemyEnter, AVent*, SelfVent );
	UPROPERTY( BlueprintAssignable, Category = "Vent" )
	FOnEnemyEnter OnEnemyEnter {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnEnemyExit, AVent*, SelfVent, AActor*, Enemy );
	UPROPERTY( BlueprintAssignable, Category = "Vent" )
	FOnEnemyExit OnEnemyExit {};

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Vent" )
	USceneComponent* DefaultSceneRoot = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Vent" )
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Vent" )
	UVentManagerComponent* VentManager = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Vent" )
	EVentZone Zone = EVentZone::A;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Vent" )
	UVentData* DataAsset = nullptr;

private:
	AParasite* SpawnEnemy( bool bAutoTargetPlayer = false );
};

