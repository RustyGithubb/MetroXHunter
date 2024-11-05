/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ParasiteAIController.generated.h"

class AParasite;
class UAIAttackerComponent;

/**
 * 
 */
UCLASS( Abstract )
class METROXHUNTER_API AParasiteAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AParasiteAIController( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;

	virtual void OnPossess( APawn* InPawn ) override;

	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void SetEnemy( AActor* Enemy );
	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	AActor* GetEnemy() const;

	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void SetInDanger( bool bInDanger );
	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	bool IsInDanger() const;

	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void SetNextVentTime( float GameTime );

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot( struct FVisualLogEntry* Snapshot ) const override;
#endif

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Parasite" )
	UAIAttackerComponent* AttackerComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Parasite" )
	UBehaviorTree* BehaviorTree = nullptr;

	UPROPERTY( VisibleDefaultsOnly, BlueprintReadOnly, Category = "Parasite" )
	AParasite* CustomPawn = nullptr;

private:
	UFUNCTION()
	void OnSeePawn( APawn* SeenPawn );
	UFUNCTION()
	void OnHearNoise( APawn* HeardPawn, const FVector& Location, float Volume );

	UFUNCTION()
	void OnGroupPlaceChanged( int32 LastGroupIndex, int32 NewGroupIndex );
};
