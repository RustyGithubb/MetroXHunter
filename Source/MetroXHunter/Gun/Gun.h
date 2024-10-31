/*
 * Implemented by Corentin Paya
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

UCLASS()
class METROXHUNTER_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	AGun();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
