/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Gun/BaseGun.h"
#include "Gun.generated.h"

class UCameraShakeSourceComponent;
class USkeletalMeshComponent;
class UNiagaraComponent;
class USoundBase;
class UGunData;
class UReloadComponent;
class UHealthComponent;
class AMetroPlayerCharacter;
class UAnimationAsset;
class UGunSoundManagerComponent;

UENUM( BlueprintType )
enum class ELightningState : uint8
{
	Charging,
	Active,

	Idle,
};

UCLASS()
class METROXHUNTER_API AGun : public ABaseGun
{
	GENERATED_BODY()

public:
	AGun();
	virtual void BeginPlay() override;
	void LateBeginPlay();
	virtual void Tick( float DeltaTime ) override;

public:
	virtual void StopLightning() override;

	UFUNCTION( BlueprintCallable, Category = "Gun" )
	void SwitchWeapon();

	UFUNCTION( BlueprintCallable, Category = "Gun|ShootAbility" )
	void TriggerShootAbility( UPARAM( ref ) FVector& ImpactDirection );

	UFUNCTION( BlueprintCallable, Category = "Gun|LightningAbility" )
	void OnLightningStart();
	UFUNCTION( BlueprintCallable, Category = "Gun|LightningAbility" )
	void OnLightningAbility( float ActionValue );
	UFUNCTION( BlueprintCallable, Category = "Gun|LightningAbility" )
	void OnLightningEnd();
	UFUNCTION( BlueprintCallable, Category = "Gun|LightningAbility" )
	void OnChargeStop();

	UFUNCTION( BlueprintImplementableEvent, category = "Gun|LightningAbility" )
	void DeactivateAllEmitters();

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Gun" )
	USceneComponent* SceneRoot = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Gun" )
	UCameraShakeSourceComponent* CameraShakeSourceComponent = nullptr;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Gun" )
	USkeletalMeshComponent* WeaponSkeletonMesh = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Gun" )
	ELightningState LightningState = ELightningState::Idle;

protected:
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable )
	void CheckLineCollision(
		bool bShouldTraceComplex,
		UPARAM( ref ) FVector& ImpactDirection,
		float Range,
		FVector& ImpactionPoint,
		FHitResult& HitResult
	);
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable )
	void CheckSphereCollision(
		bool bUseMovementImprecision,
		float Range,
		FVector& ImpactionPoint,
		FHitResult& HitResult,
		float SphereRadius = 20.0f
	);

	UFUNCTION( BlueprintImplementableEvent )
	void GetNiagaraEffects();

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun" )
	void AddImpulseAtTrace( UPARAM( ref ) FHitResult& HitResult, float ImpulseForce );
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun" )
	void AddForceAtTrace( UPARAM( ref ) FHitResult& HitResult, float ImpulseForce );

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun|LightningAbility" )
	void RetrieveReflectedEnemies( UPARAM( ref ) FVector& ImpactionPoint );
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun|LightningAbility" )
	void TriggerLightningBeam( UPARAM( ref ) FVector& ImpactionPoint );
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun|LightningAbility" )
	void TriggerEnvironmentFlickering( );
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun|LightningAbility" )
	void PlayLightningChargeTimeline();
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun|LightningAbility" )
	void ReverseLightningChargeTimeline();

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun|LightningAbility|UI" )
	void UpdateCrossHairStopLightning();
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Gun|LightningAbility|UI" )
	void UpdateCrossHairOnLightning();

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, category = "Miscellaneous" )
	void StopCameraAnimation();

protected:
	UPROPERTY( BlueprintReadWrite, Category = "Gun|GunData|LightningAbility" )
	float LightningTimer = 0.0f;
	UPROPERTY( BlueprintReadWrite, Category = "Gun|LightningAbility" )
	TMap<AActor*, AActor*> EnemiesTargeted {};

	UPROPERTY( BlueprintReadOnly, Category = "Gun|SphereCast" )
	TArray<AActor*> ActorsToIgnore {};

	UPROPERTY( BlueprintReadOnly, Category = "Gun|References" )
	AMetroPlayerCharacter* PlayerCharacter = nullptr;
	UPROPERTY( BlueprintReadOnly, Category = "Gun|References" )
	UReloadComponent* ReloadComponent = nullptr;
	UPROPERTY( BlueprintReadWrite, Category = "Gun|References" )
	UNiagaraComponent* PreloadLightningNiagara = nullptr;
	UPROPERTY( BlueprintReadWrite, Category = "Gun|References" )
	UNiagaraComponent* LightningOrbNiagara = nullptr;

	UFUNCTION()
	void OnReload();

private:
	void GetReferences();
	void CheckCurrentTargetType();
	bool HandleCanFire();
	void UpdateDualSense();

	void RetrieveFirstBeamHit();

private:
	UHealthComponent* LastAimTarget = nullptr;
	APlayerController* PlayerController = nullptr;
	UGunSoundManagerComponent* SoundManagerComponent = nullptr;
};
