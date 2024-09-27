/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ParasiteAIController.generated.h"

class AParasite;

/**
 * 
 */
UCLASS( Abstract )
class METROXHUNTER_API AParasiteAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AParasiteAIController( const FObjectInitializer& ObjectInitializer );

	virtual void OnPossess( APawn* InPawn ) override;

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot( struct FVisualLogEntry* Snapshot ) const override;
#endif

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	UBehaviorTree* BehaviorTree = nullptr;

	UPROPERTY( VisibleDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	AParasite* CustomPawn = nullptr;
};
