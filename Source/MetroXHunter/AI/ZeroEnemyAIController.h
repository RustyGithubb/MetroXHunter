#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZeroEnemyAIController.generated.h"

UENUM( BlueprintType )
enum class EZeroEnemyAIState : uint8
{
	Idle,
	Combat,
};

/**
 * 
 */
UCLASS()
class METROXHUNTER_API AZeroEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	virtual void OnPossess( APawn* InPawn ) override;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void CombatTarget( AActor* Target );

	void SetState( EZeroEnemyAIState State );
	void SetTarget( AActor* Target );

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	UBehaviorTree* BehaviorTree = nullptr;

private:
	AActor* Target = nullptr;
	EZeroEnemyAIState CurrentState = EZeroEnemyAIState::Idle;
};
