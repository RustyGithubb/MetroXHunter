/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerMovementData.generated.h"

class UCameraAnimationSequence;

UCLASS(BlueprintType)
class METROXHUNTER_API UPlayerMovementData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	float DefaultWalkSpeed = 250.f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	float DefaultRunSpeed = 480.0f;
	/*
	 * If the player stops moving after running, they have a brief window
	 * (DelayStopStart) to resume movement and continue running.
	 * If he starts moving again after this delay, he will walk instead.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	float DelayStopStart = 0.3f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	FVector2D DefaultMouseSensitivity { 0.6f };

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	UCurveFloat* MovementInputLengthCurve = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	UCurveFloat* RotationInputLengthCurve = nullptr;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement" )
	UCurveFloat* RotationDurationCurve = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|ArmLength" )
	float IdleTargetArmLength = 170.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|ArmLength" )
	float WalkTargetArmLength = 180.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|ArmLength" )
	float RunTargetArmLength = 190.f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|ArmLength" )
	float AimTargetArmLength = 170.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|ArmOffset" )
	FVector DefaultTargetOffset = FVector( 0, 40, 0 );
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|ArmOffset" )
	FVector AimTargetOffset = FVector( 10, 60, 0 );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|CameraLag", meta = ( ClampMin = 0, ClampMax = 30 ) )
	float MovementCameraLag = 15.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|CameraLag", meta = ( ClampMin = 0, ClampMax = 30 ) )
	float AimCameraLag = 30.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|CameraLag", meta = ( ClampMin = 0, ClampMax = 30 ) )
	float MovementCameraRotationLag = 8.0f;
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|SpringArm|CameraLag", meta = ( ClampMin = 0, ClampMax = 30 ) )
	float AimCameraRotationLag = 15.0f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|Camera|Animation")
	UCameraAnimationSequence* IdleCameraAnimation = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|Camera|Animation" )
	UCameraAnimationSequence* WalkCameraAnimation = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "PlayerMovement|Camera|Animation" )
	UCameraAnimationSequence* RunCameraAnimation = nullptr;
};
