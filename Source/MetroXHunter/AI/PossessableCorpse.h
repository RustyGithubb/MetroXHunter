/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PossessableCorpse.generated.h"

class AParasite;
class AZeroEnemy;
class UZeroEnemyData;

UCLASS( Abstract )
class METROXHUNTER_API APossessableCorpse : public AActor
{
	GENERATED_BODY()

public:
	APossessableCorpse();

	virtual void BeginPlay() override;

	UFUNCTION( BlueprintCallable, Category = "PossessableCorpse" )
	bool ReserveCorpse( AParasite* Parasite );
	UFUNCTION( BlueprintCallable, Category = "PossessableCorpse" )
	bool IsReserved() const;

	UFUNCTION( BlueprintPure, Category = "PossessableCorpse" )
	AParasite* GetReserver() const;

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	USkeletalMeshComponent* MeshComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "PossessableCorpse" )
	UZeroEnemyData* DataAsset = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "PossessableCorpse" )
	TSubclassOf<AZeroEnemy> EnemyClass;

private:
	AParasite* ReservingParasite = nullptr;
};
