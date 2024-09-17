/*
 * Implemented by BARRAU Benoit
 */

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GunComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class METROXHUNTER_API UGunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UGunComponent();

protected:

	virtual void BeginPlay() override;

public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
};
