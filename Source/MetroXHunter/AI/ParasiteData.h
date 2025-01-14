/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Engine/DataAsset.h"
#include "ParasiteData.generated.h"

/*
 * Data Asset for the Parasite enemy.
 */
UCLASS( BlueprintType )
class METROXHUNTER_API UParasiteData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "General" )
	int32 MaxHealth = 20;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "General" )
	FFloatRange ScaleRange { 0.9f, 1.1f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "General" )
	UMaterialInterface* BodyMaterial = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "General" )
	UMaterialInterface* WartsMaterial = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "CentimetersPerSecond" ) )
	float WalkSpeed = 600.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement" )
	FFloatRange MovementSpeedScaleRange { 0.8f, 1.2f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "Times" ) )
	float FleeSpeedScale = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "DegreesPerSecond" ) )
	float YawRotationRate = 180.0f;

	/*
	 * Minimum amount of attackers to start a group attack.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks" )
	int32 MinAttackersToAttack = 3;

	/*
	 * Damage to apply when jump attacking.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Jump" )
	int32 JumpAttackDamage = 30;
	/*
	 * Token to reserve to the target's AITargetComponent before performing a jump attack.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Jump" )
	int32 JumpAttackToken = 1;
	/*
	 * Time to apply as a cooldown to the target's AITargetComponent
	 * after a jump attack token has been freed.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Attacks|Jump", meta = ( Units = "Seconds" ) )
	float JumpAttackTokenCooldown = 1.0f;

	/*
	 * When exiting a vent, the time to apply as a cooldown to prevent the use of a vent,
	 * forcing the parasite to flee around in the map.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Vent", meta = ( Units = "Seconds" ) )
	float ExitVentNextUseCooldown = 15.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Flee from Target" )
	FFloatRange FleeAimAdditionalDistanceRange { -300.0f, 300.0f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Flee from Target", meta = ( Units = "Centimeters", ClampMin = "0" ) )
	float FleeAimMinDistance = 500.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Flee from Target", meta = ( Units = "Degrees", ClampMin = "10", ClampMax = "179" ) )
	float FleeAimFOV = 60.0f;

	/*
	 * Collision responses of the mesh when ragdolling.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	FCollisionResponseContainer MeshRagdollCollisions {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	TSubclassOf<AActor> BloodPuddleClass {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	FVector BloodPuddleScale { 2.0f, 10.0f, 10.0f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death" )
	FName BloodPuddleSpawnBoneName = TEXT( "Body" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Death", meta = ( Units = "CentimetersPerSecond" ) )
	float BloodPuddleSpawnMaxVelocity = 1.0f;

	/*
	 * Don't disable or AIs won't be able to move anymore.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Debug" )
	bool bIsCrowdSimulationDisabled = true;
};
