/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Engine/DataAsset.h"
#include "ZeroEnemyData.generated.h"

class UQuickTimeEventData;
class UZeroEnemyAISubstate;
class ASpitProjectile;

class USoundBase;
class UCurveVector;
class UNiagaraSystem;

/**
 * 
 */
UCLASS( BlueprintType )
class METROXHUNTER_API UZeroEnemyData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "General" )
	int32 MaxHealth = 100;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "CentimetersPerSecond" ) )
	float WalkSpeed = 270.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "CentimetersPerSecond" ) )
	float WalkSpeedLossPerBodyPartLost = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "DegreesPerSecond" ) )
	float YawRotationRate = 80.0f;
	
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Rush", meta = ( Units = "DegreesPerSecond" ) )
	float RushYawRotationRate = 30.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Rush", meta = ( Units = "Centimeters" ) )
	FFloatRange RushFromDistance { 500.0f, 700.0f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Rush" )
	int32 RushDamage = 50;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Rush" )
	UCurveFloat* RushSpeedCurve = nullptr;
	/*
	 * Data asset to use for the quick time event triggering after a rush hit the player.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Rush" )
	UQuickTimeEventData* RushQuickTimeEventDataAsset = nullptr;
	/*
	 * Token to reserve to the target's AITargetComponent before performing a rush attack.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Rush" )
	int32 RushTokens = 1;
	/*
	 * Time to apply as a cooldown to the target's AITargetComponent
	 * after a rush token has been freed.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Rush", meta = ( Units = "Seconds" ) )
	float RushTokenCooldown = 2.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Melee" )
	int32 MeleeDamage = 25;
	/*
	 * Token to reserve to the target's AITargetComponent before performing a melee attack.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Melee" )
	int32 MeleeTokens = 1;
	/*
	 * Time to apply as a cooldown to the target's AITargetComponent 
	 * after a melee token has been freed.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Melee", meta = ( Units = "Seconds" ) )
	float MeleeTokenCooldown = 1.5f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Spit" )
	TSubclassOf<ASpitProjectile> SpitProjectileClass {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Spit" )
	float SpitSpreadRange = 300.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Scream" )
	FFloatRange ScreamTimeRange { 3.0f, 5.0f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Scream" )
	float ScreamFlickeringLightRadius = 1024.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Scream" )
	UCurveVector* ScreamFlickeringLightCurve = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Substates", meta = ( EditCondition = "false" ) )
	TArray<TSubclassOf<UZeroEnemyAISubstate>> SubstateClasses {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Substates", meta = ( EditCondition = "false" ) )
	float SubstateChangeAnimationTime = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "Seconds" ) )
	float PanicBulbOpenTime = 5.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "Seconds" ) )
	float PanicStunTime = 5.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic" )
	bool bPanicOnlyIfDismembered = true;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|Stun", meta = ( Units = "Degrees" ) )
	float StunAnimationAngle = 12.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|Stun" )
	float StunAnimationFrequency = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|SubstateSwitched", meta = ( Units = "Degrees" ) )
	float SubstateSwitchedAnimationAngle = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|SubstateSwitched" )
	float SubstateSwitchedAnimationFrequency = 1.0f;

	/*
	 * Amount of body parts left to force kill the enemy.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BodyParts" )
	int32 BodyPartsLeftToKill = 1;
	/*
	 * Collision responses to apply for all body parts during BeginPlay.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BodyParts" )
	FCollisionResponseContainer BodyPartDefaultCollisions {};
	/*
	 * Collision responses to apply for body parts becoming ragdoll.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BodyParts" )
	FCollisionResponseContainer BodyPartRagdollCollisions {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BodyParts", meta = ( EditCondition = "false" ) )
	bool bBodyPartHasAimAssist = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Collisions" )
	TEnumAsByte<ECollisionChannel> AimAssistCollisionChannel;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Collisions" )
	TEnumAsByte<ECollisionChannel> DefaultBodyPartCollisionChannel;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float DefaultKnockbackZ = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float MeleeKnockbackForce = 300.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float BodyPartHitKnockbackForce = 1500.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float WholeBodyHitKnockbackForce = 500.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float QuickTimeEventSucceedKnockbackForce = 1250.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "KnockOut" )
	UCurveFloat* KnockOutKnockbackCurve = nullptr;


	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	FCollisionResponseContainer MeshRagdollCollisions {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death", meta = ( Units = "Centimeters" ) )
	float DeathKnockbackForce = 500.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	TSubclassOf<AActor> BloodPuddleClass {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	FVector BloodPuddleScale { 2.0f, 14.0f, 14.0f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	FName BloodPuddleSpawnBoneName = TEXT( "spine_02" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death", meta = ( Units = "CentimetersPerSecond" ) )
	float BloodPuddleSpawnMaxVelocity = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Materials" )
	UMaterialInterface* OpenedBulbMaterial = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Materials" )
	UMaterialInterface* ClosedBulbMaterial = nullptr;

	/*
	 * Niagara system to spawn when hitting the bulb.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Niagara" )
	UNiagaraSystem* BulbHitNiagara = nullptr;
	/*
	 * Forward distance to spawn the Niagara system when hitting the bulb.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Niagara", meta = ( Units = "Centimeters" ) )
	float BulbHitNiagaraDistance = 50.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundBase* RushStartSound = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundBase* FootstepSound = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundBase* ScreamSound = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundConcurrency* SoundConcurrency = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundAttenuation* SoundAttenuation = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tags" )
	FName BulbSpotTag = TEXT( "BulbSpot" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tags" )
	FName BodyPartTag = TEXT( "BodyPart" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Debug" )
	bool bIsCrowdSimulationDisabled = false;
};
