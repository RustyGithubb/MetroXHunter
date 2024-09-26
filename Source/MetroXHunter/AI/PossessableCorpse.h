/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PossessableCorpse.generated.h"

class AZeroEnemy;
class UZeroEnemyData;

UCLASS( Abstract )
class METROXHUNTER_API APossessableCorpse : public AActor
{
	GENERATED_BODY()

public:
	APossessableCorpse();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	USceneComponent* SceneComponent;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	UStaticMeshComponent* MeshComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "PossessableCorpse" )
	UZeroEnemyData* DataAsset;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "PossessableCorpse" )
	TSubclassOf<AZeroEnemy> EnemyClass;
};
