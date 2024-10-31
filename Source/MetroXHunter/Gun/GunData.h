/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "Engine/DataAsset.h"
#include "GunData.generated.h"

 /*
  * Data Asset of the gun.
  */
UCLASS()
class METROXHUNTER_API UGunData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ShootAbility" )
	int ShootDamage = 50;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ShootAbility" )
	int StartingMagazineAmmo = 6;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ShootAbility" )
	int MaxMagazineAmmo = 6;

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
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "LightningAbility" )
	int LightningTargetLimit = 3;

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
