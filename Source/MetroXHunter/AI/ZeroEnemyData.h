#pragma once

#include "Engine/DataAsset.h"
#include "ZeroEnemyData.generated.h"

class UAISubstate;
class USoundBase;
class UQuickTimeEventData;

/**
 * 
 */
UCLASS()
class METROXHUNTER_API UZeroEnemyData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "CentimetersPerSecond" ) )
	float WalkSpeed = 270.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "CentimetersPerSecond" ) )
	float WalkSpeedLossPerBodyPartLost = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "DegreesPerSecond" ) )
	float YawRotationRate = 80.0f;

	// Deprecated: now use RushSpeedCurve
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Rush", meta = ( Units = "CentimetersPerSecond" ) )
	float RushSpeed = 500.0f;
	// Deprecated: now use RushSpeedCurve
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Rush", meta = ( Units = "Seconds" ) )
	float RushTime = 2.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rush", meta = ( Units = "DegreesPerSecond" ) )
	float RushYawRotationRate = 30.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rush", meta = ( Units = "Centimeters" ) )
	FFloatRange RushFromDistance { 500.0f, 700.0f };
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rush" )
	int32 RushDamage = 50;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rush" )
	UCurveFloat* RushSpeedCurve = nullptr;
	/*
	 * Data asset to use for the quick time event triggering after a rush hit the player.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Rush" )
	UQuickTimeEventData* RushQuickTimeEventDataAsset = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Melee" )
	int32 MeleeDamage = 25;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Substates" )
	TArray<TSubclassOf<UAISubstate>> SubstateClasses;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "Seconds" ) )
	float PanicBulbOpenTime = 5.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "Seconds" ) )
	float PanicStunTime = 5.0f;

	/*
	 * Time to apply as a cooldown to the target's AITargetComponent
	 * after a rush token has been freed.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "AI|Tokens", meta = ( Units = "Seconds" ) )
	float RushTokenCooldown = 2.0f;
	/*
	 * Time to apply as a cooldown to the target's AITargetComponent 
	 * after a melee token has been freed.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "AI|Tokens", meta = ( Units = "Seconds" ) )
	float MeleeTokenCooldown = 1.5f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Debug" )
	bool bIsCrowdSimulationDisabled = false;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|Stun", meta = ( Units = "Degrees" ) )
	float StunAnimationAngle = 12.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|Stun" )
	float StunAnimationFrequency = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|SubstateSwitched", meta = ( Units = "Degrees" ) )
	float SubstateSwitchedAnimationAngle = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Animation|SubstateSwitched" )
	float SubstateSwitchedAnimationFrequency = 1.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BodyParts" )
	int32 BodyPartsLeftToKill = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Collisions" )
	TEnumAsByte<ECollisionChannel> AimAssistCollisionChannel;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Collisions" )
	TEnumAsByte<ECollisionChannel> DefaultBodyPartCollisionChannel;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float DefaultKnockbackZ = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float BodyPartHitKnockbackForce = 1500.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float WholeBodyHitKnockbackForce = 500.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float QuickTimeEventSucceedKnockbackForce = 1250.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Materials" )
	UMaterialInterface* OpenedBulbMaterial = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Materials" )
	UMaterialInterface* ClosedBulbMaterial = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundBase* RushStartSound = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundBase* FootstepSound = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sounds" )
	USoundBase* SubstateChangedSound = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tags" )
	FName BulbSpotTag = TEXT( "BulbSpot" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tags" )
	FName BodyPartTag = TEXT( "BodyPart" );
};
