/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerMovementData.generated.h"

UCLASS()
class METROXHUNTER_API UPlayerMovementData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	float DefaultWalkSpeed = 250.f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	float DefaultRunSpeed = 480.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	FVector2D DefaultMouseSensitivity { 0.6f };

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	UCurveFloat* MovementCurve = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	UCurveFloat* RotationCurve = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm" )
	float IdleTargetArmLength = 170.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm" )
	float WalkTargetArmLength = 180.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm" )
	float RunTargetArmLength = 190.f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm" )
	float AimTargetArmLength = 170.0f;
};
