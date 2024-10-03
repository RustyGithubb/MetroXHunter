/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "Engine/DataAsset.h"
#include "LockerData.generated.h"

 /*
  * Data asset for the Locker and its widget.
  */
UCLASS()
class METROXHUNTER_API ULockerData : public UDataAsset
{
	GENERATED_BODY()

public:
	/* 
	 * The time limit to unlock the locker
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = ( Units = "Seconds" ) )
	float TimeLimit = 6.0f;

	/*
	 * The cooldown between two press attempts
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = ( Units = "Seconds" ) )
	float PressCooldown = 0.3f;

	/* 
	 * The cooldown penalty when the sequence is wrong
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = ( Units = "Seconds" ) )
	float PressCooldownPenalty = 0.4f;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sound" )
	USoundBase* SequenceFailedSound;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sound" )
	USoundBase* CorrectPressSound;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Sound" )
	USoundBase* TickSound;

	/* 
	 * How fast the jostick smoothly snap to the target
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "RotationSpeed" )
	float JoystickSlerpSpeed = 40.0f;

	/* 
	 * The size of the background circle
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BackgroundCircle" )
	float TextureSize = 250.0f;

	/*
	 * How thick is the background circle line
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BackgroundCircle" )
	float LineThickness = 6.0f;

	/* 
	 * How sharp is the background circle line;
	 * A lower value means that the circle line is sharper 
	 * A higher value means that the circle line is more blurry
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "BackgroundCircle" )
	float LineSharpness = 6.0f;
};
