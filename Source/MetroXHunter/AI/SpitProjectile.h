/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpitProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UHealthComponent;

UCLASS()
class METROXHUNTER_API ASpitProjectile : public AActor
{
	GENERATED_BODY()

public:
	ASpitProjectile();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "SpitProjectile" )
	void ComputeVelocityToLocation( const FVector& TargetLocation, float SpreadRange );
	UFUNCTION( BlueprintCallable, Category = "SpitProjectile" )
	void Explode();

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "SpitProjectile" )
	USphereComponent* SphereComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "SpitProjectile" )
	UStaticMeshComponent* MeshComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "SpitProjectile" )
	UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "SpitProjectile" )
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SpitProjectile" )
	float LaunchSpeed = 1000.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SpitProjectile" )
	float ExplosionRadius = 100.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SpitProjectile" )
	int32 ExplosionDamage = 25;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SpitProjectile" )
	TArray<TEnumAsByte<EObjectTypeQuery>> ExplosionObjectTypes {};

private:
	UFUNCTION()
	void OnProjectileStop( const FHitResult& ImpactResult );
	UFUNCTION()
	void OnDeath( const FDamageContext& DamageContext );
};
