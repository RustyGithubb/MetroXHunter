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
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "cm" ) )
	float WalkSpeed = 270.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = ( Units = "cm" ) )
	float WalkSpeedLossPerBodyPartLost = 20.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "s" ) )
	float PanicBulbOpenTime = 5.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Panic", meta = ( Units = "s" ) )
	float PanicStunTime = 5.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "StunAnimation", meta = ( Units = "deg" ) )
	float StunAnimationAngle = 45.0f; 
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "StunAnimation" )
	float StunAnimationFrequency = 6.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BodyParts" )
	int32 BodyPartsLeftToKill = 1;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "cm" ) )
	float DefaultKnockbackZ = 20.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Knockback", meta = ( Units = "cm" ) )
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
