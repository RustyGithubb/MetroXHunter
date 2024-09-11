#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZeroEnemyAIController.generated.h"

class AZeroEnemy;

UENUM( BlueprintType )
enum class EZeroEnemyAIState : uint8
{
	Idle,
	Combat,
	Stun,
	RushAttack,
};

/**
 * 
 */
UCLASS()
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
};
