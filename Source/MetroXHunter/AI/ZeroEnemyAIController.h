/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI/EQSContextProvider.h"

#include "ZeroEnemyAIController.generated.h"

enum class EZeroEnemyState : uint8;
class AZeroEnemy;
class UAIAttackerComponent;
class UAISubstateManagerComponent;

UENUM( BlueprintType )
enum class EZeroEnemyAIState : uint8
{
	Idle,
	/*
	 * Lock a target before chasing it
	 */
	Target,
	/*
	 * Chasing a target and deciding which attack to use.
	 */
	Chase,
	/*
	 * State of paralysis for a short time.
	 */
	Stun,
	/*
	 * Heavy and long-range attack by charging the target and dealing high damage.
	 */
	RushAttack,
	/*
	 * Light and close-range attack.
	 */
	MeleeAttack,
	/*
	 * Area of effect and long-range distance attack.
	 */
	SpitAttack,
	/*
	 * Spawn parasites from the vents around the player.
	 */
	CallForParasites,
};

/**
 * 
 */
UCLASS( Abstract )
class METROXHUNTER_API AZeroEnemyAIController : public AAIController, public IEQSContextProvider
{
	GENERATED_BODY()
	
public:
	AZeroEnemyAIController( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	virtual void OnPossess( APawn* InPawn ) override;
	virtual void OnUnPossess() override;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void CombatTarget( AActor* Target );

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SetState( EZeroEnemyAIState State );
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "ZeroEnemy" )
	EZeroEnemyAIState GetState() const;
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	bool SetTarget( AActor* NewTarget );
	UFUNCTION( BlueprintPure, Category = "ZeroEnemy" )
	AActor* GetTarget() const;

	/*
	 * Returns the madness level, representing the current progress of the AISubstateManagerComponent.
	 * In range of 0.0f to 1.0f.
	 */
	UFUNCTION( BlueprintPure, Category = "ZeroEnemy" )
	float GetMadnessLevel() const;

	// Begin IEQSContextProvider interface
	FVector GetEQSStartLocation_Implementation() const override;
	AActor* GetEQSTargetActor_Implementation() const override;
	// End IEQSContextProvider interface

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot( struct FVisualLogEntry* Snapshot ) const override;
#endif

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UAIAttackerComponent* AttackerComponent = nullptr;
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UAISubstateManagerComponent* SubstateManagerComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	UBehaviorTree* BehaviorTree = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "ZeroEnemy" )
	AZeroEnemy* CustomPawn = nullptr;

private:
	void InitializeAISubstateManager();

	void TickDebugDraw();

	void OnScreamUpdate();
	void StartScreamTimer();
	void StopScreamTimer();

	UFUNCTION()
	void OnTargetDeath( const FDamageContext& DamageContext );

	UFUNCTION()
	void OnSeePawn( APawn* SeenPawn );
	UFUNCTION()
	void OnHearNoise( APawn* HeardPawn, const FVector& Location, float Volume );

	UFUNCTION()
	void OnStun();
	UFUNCTION()
	void OnUnStun();

	UFUNCTION()
	void OnRush();
	UFUNCTION()
	void OnUnRush();

	UFUNCTION()
	void OnStateUpdate( EZeroEnemyState NewState, EZeroEnemyState OldState );

	UFUNCTION()
	void OnAttacked( AActor* Attacker );

	UFUNCTION()
	void OnSubstateSwitched();

private:
	FTimerHandle ScreamTimerHandle {};
	FVector SpawnLocation = FVector::ZeroVector;
};
