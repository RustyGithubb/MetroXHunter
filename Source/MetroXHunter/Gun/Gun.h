/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

class UCameraShakeSourceComponent;
class USkeletalMeshComponent;
class UGunData;
class UReloadComponent;
class AMetroPlayerCharacter;

UENUM( BlueprintType )
enum class EGunMode : uint8
{
	Bullet,
	Lightning,
};

UCLASS()
class METROXHUNTER_API AGun : public AActor
{
	GENERATED_BODY()

public:
	AGun();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION( BlueprintCallable, BlueprintPure )
	bool CanFire();

public:
	UFUNCTION( BlueprintCallable )
	void SwitchWeapon();
	UFUNCTION( BlueprintCallable )
	void TriggerShootAbility( UPARAM( ref ) FVector& ImpactDirection );
	UFUNCTION( BlueprintCallable )
	void OnLightningStart();
	UFUNCTION( BlueprintCallable )
	void OnLightningAbility();
	UFUNCTION( BlueprintCallable )
	void OnLightningEnd();

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Gun" )
	USceneComponent* SceneRoot = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Gun" )
	USceneComponent* ShootPoint = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Gun" )
	UCameraShakeSourceComponent* CameraShakeSourceComponent = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Gun" )
	USkeletalMeshComponent* WeaponSkeletonMesh = nullptr;

	UPROPERTY( BlueprintReadWrite, Category = "Gun|GunMode" )
	EGunMode GunMode = EGunMode::Bullet;
	UPROPERTY( BlueprintReadWrite, Category = "Gun|GunData" )
	UGunData* GunData = nullptr;
	UPROPERTY( BlueprintReadWrite, Category = "Gun|References" )
	UReloadComponent* ReloadComponent = nullptr;

protected:
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable )
	void CheckLineCollision( bool bUseMovementImprecision, UPARAM( ref ) FVector& ImpactDirection, FVector& ImpactionPoint, FHitResult& HitResult );
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable )
	void CheckSphereCollision( 
		bool bUseMovementImprecision,
		FVector& ImpactionPoint,
		FHitResult& HitResult,
		float SphereRadius = 20.0f
	);

protected:
	UPROPERTY( BlueprintReadOnly, Category = "Gun|GunData|ShootAbility" )
	float ShootCurentCooldown = 0.0f;

	UPROPERTY( BlueprintReadOnly, Category = "Gun|GunData|LightningAbility" )
	bool bIsLightningCharged = false;
	UPROPERTY( BlueprintReadOnly, Category = "Gun|GunData|LightningAbility" )
	float CurrentEnergyAmount = 0;
	UPROPERTY( BlueprintReadOnly, Category = "Gun|SphereCast" )
	TArray<AActor*> ActorsToIgnore;

	UPROPERTY( BlueprintReadOnly, Category = "Gun|References" )
	AMetroPlayerCharacter* PlayerCharacter = nullptr;

private:
	void GetReferences();
};
