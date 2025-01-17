/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseGun.generated.h"

class UHealthComponent;
class UGunData;

UENUM( BlueprintType )
enum class EGunMode : uint8
{
	Bullet,
	Lightning,
};

UCLASS()
class METROXHUNTER_API ABaseGun : public AActor
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnLightningShooting, float, Offset, float, Alpha );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "Gun|Event|LightningAbility" )
	FOnLightningShooting OnLightningShooting;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnLightningStop, float, Offset, float, Alpha );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "Gun|Event|LightningAbility" )
	FOnLightningStop OnLightningStop;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnLightningStart );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "Gun|Event|LightningAbility" )
	FOnLightningStart OnLightningStart;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnBulletShoot );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "Gun|Event|ShootAbility" )
	FOnBulletShoot OnBulletShoot;
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAimTargetChanged, UHealthComponent*, HealthComponent );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "Gun|Event|ShootAbility" )
	FOnAimTargetChanged OnAimTargetChanged;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnToggleWeaponMode, EGunMode, GunMode );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "Gun|Event|Springfield" )
	FOnToggleWeaponMode OnToggleWeaponMode;

	UPROPERTY( BlueprintReadWrite, Category = "Gun|GunData" )
	UGunData* GunData = nullptr;

	UPROPERTY( BlueprintReadWrite, Category = "Gun|GunMode" )
	EGunMode GunMode = EGunMode::Bullet;

	UPROPERTY( BlueprintReadWrite, Category = "Gun|LightningAbility" )
	float CurrentEnergyAmount = 0;
	UPROPERTY( BlueprintReadWrite, Category = "Gun|LightningAbility" )
	float LightningChargeValue = 0.0f;
	UPROPERTY( BlueprintReadWrite, Category = "Gun|LightningAbility" )
	float ChargingWeight = 1.0f;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Gun" )
	USceneComponent* ShootPoint = nullptr;
};
