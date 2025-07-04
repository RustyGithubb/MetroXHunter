/* 
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "AI/AISubstate.h"
#include "ZeroEnemyAISubstate.generated.h"

class AZeroEnemyAIController;

UCLASS()
class METROXHUNTER_API UZeroEnemyAISubstateData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "CentimetersPerSecond" ) )
	float WalkSpeed = 500.0f;
};

/*
 * 
 */
UCLASS( Abstract )
class METROXHUNTER_API UZeroEnemyAISubstate : public UAISubstate
{
	GENERATED_BODY()
	
public:
	void OnSubstateInitialized_Implementation() override;
	void OnSubstateEntered_Implementation() override;
	void OnSubstateExited_Implementation() override;

public:
	UPROPERTY( BlueprintReadOnly, Category = "ZeroEnemyAISubstate" )
	AZeroEnemyAIController* AIController;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ZeroEnemyAISubstate" )
	UZeroEnemyAISubstateData* DataAsset = nullptr;
};
