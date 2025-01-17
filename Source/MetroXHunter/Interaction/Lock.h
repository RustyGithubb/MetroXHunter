/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Electricity/ElectrocutableComponent.h"
#include "Health/HealthComponent.h"
#include "Lock.generated.h"

class UHealthComponent;

/*
 * A lock with a HealthComponent and an OnDeath event to control doors and other interactable objects.
 */
UCLASS()
class METROXHUNTER_API ALock : public AActor, public IHealthHolder
{
	GENERATED_BODY()

public:
	ALock();

	void BeginPlay() override;

	// Begin IHealthHolder interface
	bool TakeDamage_Implementation( UPARAM( ref ) FDamageContext& DamageContext ) override;
	bool IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType ) override;
	// End IHealthHolder interface

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLockDown, ALock*, Lock );
	UPROPERTY( BlueprintAssignable )
	FOnLockDown OnLockDown;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	UStaticMeshComponent* StaticMesh = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	UHealthComponent* HealthComponent = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	UElectrocutableComponent* ElectrocutableComponent = nullptr;

private:
	UFUNCTION()
	void OnDeath( const FDamageContext& DamageContext );
};
