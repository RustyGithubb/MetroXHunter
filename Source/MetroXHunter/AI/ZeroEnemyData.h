#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZeroEnemyData.generated.h"

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

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "Seconds" ) )
	float PanicBulbOpenTime = 5.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "Seconds" ) )
	float PanicStunTime = 5.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "StunAnimation", meta = ( Units = "Degrees" ) )
	float StunAnimationAngle = 45.0f; 
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "StunAnimation" )
	float StunAnimationFrequency = 6.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BodyParts" )
	int32 BodyPartsLeftToKill = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Collisions" )
	TEnumAsByte<ECollisionChannel> AimAssistCollisionChannel;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Collisions" )
	TEnumAsByte<ECollisionChannel> DefaultBodyPartCollisionChannel;


	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float DefaultKnockbackZ = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "Centimeters" ) )
	float BodyPartHitKnockbackForce = 500.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Materials" )
	UMaterialInterface* OpenedBulbMaterial = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Materials" )
	UMaterialInterface* ClosedBulbMaterial = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tags" )
	FName BulbSpotTag = TEXT( "BulbSpot" );
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Tags" )
	FName BodyPartTag = TEXT( "BodyPart" );
};
