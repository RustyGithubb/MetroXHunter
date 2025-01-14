/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "Engine/DataAsset.h"
#include "GunData.generated.h"

class USoundBase;
class UNiagaraSystem;

 /*
  * Data Asset of the gun.
  */
UCLASS( Blueprintable )
class METROXHUNTER_API UGunData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ShootAbility|Damage" )
	int32 ShootDamage = 50;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ShootAbility|Damage" )
	float ShootCooldown = 1.2f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ShootAbility" )
	int32 StartingMagazineAmmo = 6;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ShootAbility" )
	int32 MaxMagazineAmmo = 6;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|Sound" )
	USoundBase* ShootFailedSound;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|Sound" )
	USoundBase* CantShootSound;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|Sound" )
	USoundBase* ShootSound;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|FX" )
	TSubclassOf<AActor> BloodBulletDecal;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|FX" )
	TSubclassOf<AActor> BulletDecal;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|FX" )
	UNiagaraSystem* MuzzleFlashNiagaraSystem;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|FX" )
	UNiagaraSystem* StructurImpactNiagaraSystem;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ShootAbility|FX" )
	UNiagaraSystem* BloodImpactNiagaraSystem;

	/*
 * The cooldown time before Lightning Damage can be reapplied to the targets.
 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Damage", meta = ( Units = "Seconds" ) )
	float DamageCooldown = 0.2f;
	/*
	 * Note: Damage is not dealt per second but is based on the DamageCooldown value.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Damage" )
	int32 LightningDamage = 15;

	/*
	 * The range of the lightning ability:
	 * The maximum distance from the player to the primary target for the lightning to hit.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility" )
	float LightningDistance = 800.0f;
	/*
	 * The maximum distance from a lightning-struck target to the next one.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility" )
	float ReflectionDistance = 150.0f;
	/*
	 * The maximum number of lightning-struck targets.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility", meta = ( ClampMin = 1, ClampMax = 3 ) )
	int32 LightningTargetLimit = 3;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Sound" )
	USoundBase* LightningAbilityOn;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Sound" )
	USoundBase* LightningAbilityOff;

	/*
	 * The maximum stun duration of the lightning
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Stun" )
	float LightningMaxStunDuration = 5.0f;
	/*
	 * The minimum stun duration of the lightning
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Stun" )
	float LightningMinStunDuration = 0.5f;
	/*
	 * @brief The stun duration scales with the ChargeWeight and LightningTimer values:
	 * higher values lead to a faster reach of maximum stun duration.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Stun" )
	float StunWeightMultiplier = 2.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Energy" )
	float MaxEnergyAmount = 100.0f;
	/*
	 * The cost on energy per seconds of the Lightning Ability.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Energy" )
	float EnergyCost = 25.0f;
	/*
	 * The passive regeneration of the energy per seconds.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Energy" )
	float PassiveRegeneration = 10.0f;
	/*
	 * The cooldown time before passive regeneration starts again after that the Lightning Ability stops.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility|Energy", meta = ( Units = "Seconds" ) )
	float PassiveRegenerationCooldown = 1.0f;
};
