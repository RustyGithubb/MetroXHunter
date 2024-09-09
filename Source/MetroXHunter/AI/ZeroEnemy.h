#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
	UPROPERTY( EditAnywhere )
	UStaticMeshComponent* ProtoMeshComponent;
};
