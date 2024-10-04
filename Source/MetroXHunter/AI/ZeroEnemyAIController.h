/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZeroEnemyAIController.generated.h"

class AZeroEnemy;

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
};

/**
 * 
 */
UCLASS( Abstract )
class METROXHUNTER_API AZeroEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AZeroEnemyAIController( const FObjectInitializer& ObjectInitializer );

	virtual void OnPossess( APawn* InPawn ) override;
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void CombatTarget( AActor* Target );

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SetState( EZeroEnemyAIState State );
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "ZeroEnemy" )
	EZeroEnemyAIState GetState() const;
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SetTarget( AActor* Target );
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "ZeroEnemy" )
	AActor* GetTarget() const;

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot( struct FVisualLogEntry* Snapshot ) const override;
#endif

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	UBehaviorTree* BehaviorTree = nullptr;

	UPROPERTY( VisibleDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	AZeroEnemy* CustomPawn = nullptr;

private:
	void TickDebugDraw();

	UFUNCTION()
	void OnStun();
	UFUNCTION()
	void OnUnStun();

	UFUNCTION()
	void OnRush();
	UFUNCTION()
	void OnUnRush();

	UFUNCTION()
	void OnStateUpdate();
};
