/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SoundAtmosphere.generated.h"

class AAmbientSound;

/*
 * Enum representing an action type for a SoundAtmosphere component.
 */
UENUM( BlueprintType )
enum class ESoundAtmosphereActionType : uint8
{
	/*
	 * Play sound when entering the trigger box and stop it when exiting.
	 */
	PlayAndStop,
	/*
	 * Play sound when entering the trigger box and trigger a sound parameter when exiting.
	 */
	PlayAndTrigger,
	/*
	 * Play sound when entering the trigger box.
	 */
	Play,
	/*
	 * Trigger a sound parameter when entering the trigger box.
	 */
	Trigger,
	/*
	 * Stop a sound when entering the trigger box.
	 */
	Stop,
};

/*
 * Structure containing all variables for a SoundAtmosphere component to holds.
 * It is created in C++ for the benefits of using EditCondition.
 */
USTRUCT( BlueprintType )
struct FSoundAtmosphereDetails
{
	GENERATED_BODY()

	/*
	 * Specify how the component behaves.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundAtmosphere" )
	ESoundAtmosphereActionType Action = ESoundAtmosphereActionType::PlayAndStop;
	/*
	 * Reference to an AmbientSound actor serving as a proxy for multiple trigger boxes to communicate with.
	 * This allows to play 3D sounds.
	 * If set, it is in this actor that you have to configure its parameters.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundAtmosphere", meta = ( EditCondition = "SoundAsset==nullptr", EditConditionHides ) )
	AAmbientSound* AmbientSound = nullptr;

	/*
	 * If no reference to an ambient sound is set, the component will play the given sound asset on its own.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundAtmosphere|Play", meta = ( EditCondition = "(Action==ESoundAtmosphereActionType::Play || Action==ESoundAtmosphereActionType::PlayAndStop || Action==ESoundAtmosphereActionType::PlayAndTrigger) && AmbientSound==nullptr", EditConditionHides ) )
	USoundBase* SoundAsset = nullptr;
	UPROPERTY( BlueprintReadOnly, Category = "SoundAtmosphere|Play", meta = ( EditCondition = "(Action==ESoundAtmosphereActionType::Play || Action==ESoundAtmosphereActionType::PlayAndStop || Action==ESoundAtmosphereActionType::PlayAndTrigger) && AmbientSound==nullptr", EditConditionHides ) )
	bool bIsPlaying2D = true;

	/*
	 * Sound parameter to trigger. This allows to trigger MetaSounds inputs.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundAtmosphere|Trigger", meta = ( EditCondition = "Action==ESoundAtmosphereActionType::Trigger || Action==ESoundAtmosphereActionType::PlayAndTrigger", EditConditionHides ) )
	FName TriggerParameter = NAME_None;

	/*
	 * Time in seconds used to fade out the sound.
	 * If set below 0.0, this will instantly stop the sound instead.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundAtmosphere|Stop", meta = ( Units = "Seconds", EditCondition = "Action==ESoundAtmosphereActionType::Stop || Action==ESoundAtmosphereActionType::PlayAndStop", EditConditionHides ))
	float FadeOutTime = -1.0f;
};

/*
 * Enum representing an action type for a SoundScape component.
 */
UENUM( BlueprintType )
enum class ESoundScapeActionType : uint8
{
	/*
	 * Set the in soundscape state while the player is inside the trigger box.
	 */
	Inside,
	/*
	 * Set and clear a soundscape state depending on which direction is exiting the trigger box.
	 */
	ExitPlayerDirection,
};

/*
 * Structure containing all variables for a SoundScape component to holds.
 * It is created in C++ for the benefits of using EditCondition.
 */
USTRUCT( BlueprintType )
struct FSoundScapeDetails
{
	GENERATED_BODY()

	/*
	 * Specify how the component behaves.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundScape" )
	ESoundScapeActionType Action = ESoundScapeActionType::Inside;

	/*
	 * Soundscape state to apply.
	 * With action type set to Inside, it is the state set while the player is inside the trigger box.
	 * With action type set to ExitPlayerDirection, it is the state set if the player is going in the backward direction of the trigger box.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundScape" )
	FGameplayTag InState;

	/*
	 * Soundscape state to apply.
	 * With action type set to Inside, it is the state cleared when the player enters the trigger box and set back when he exits it.
	 * With action type set to ExitPlayerDirection, it is the state set if the player is going in the forward direction of the trigger box.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "SoundScape" )
	FGameplayTag OutState;
};