/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Lock.generated.h"

class UHealthComponent;

UCLASS()
class METROXHUNTER_API ALock : public AActor
{
	GENERATED_BODY()
	
public:	
	ALock();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLockDown, ALock*, Lock );
	FOnLockDown OnLockDown;

public:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY( EditAnywhere )
	UHealthComponent* HealthComponent = nullptr;

private:
	UFUNCTION()
	void OnDeath( FDamageContext DamageContext );
};
