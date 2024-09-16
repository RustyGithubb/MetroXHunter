#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/ZeroEnemyData.h"
#include "ZeroEnemy.generated.h"

UCLASS()
class METROXHUNTER_API AZeroEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AZeroEnemy();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	UStaticMeshComponent* ProtoMeshComponent;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ZeroEnemy" )
	UZeroEnemyData* Data;

private:
	void RetrieveReferences();
};
