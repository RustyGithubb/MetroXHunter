/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "LatentActions.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuickTimeEventBlueprintLibrary.generated.h"

class UQuickTimeEventComponent;
class UQuickTimeEventData;

/*
 * All output pins for the quick time event latent blueprint function.
 */
UENUM()
enum class EQuickTimeEventOutputPins : uint8
{
	/*
	 * When the player succeed the event.
	 */
	OnSucceed,
	/*
	 * When the player failed the event.
	 */
	OnFailed,
};

/*
 * Handles the latent blueprint function for the quick time event.
 * See UQuickTimeEventBlueprintLibrary::LatentQuickTimeEvent for more info.
 */
class FLatentQuickTimeEvent : public FPendingLatentAction
{
public:
	FLatentQuickTimeEvent(
		FLatentActionInfo& LatentInfo, EQuickTimeEventOutputPins& OutputPins,
		UQuickTimeEventComponent* Component,
		UQuickTimeEventData* DataAsset,
		AActor* Inflictor
	)
		: LatentInfo( LatentInfo ), OutputPins( OutputPins ),
		  Component( Component ), DataAsset( DataAsset ), Inflictor( Inflictor )
	{}

	virtual void UpdateOperation( FLatentResponse& Response ) override;

public:
	bool bIsInitialized = false;

	FLatentActionInfo LatentInfo {};
	EQuickTimeEventOutputPins& OutputPins;

	UQuickTimeEventComponent* Component = nullptr;
	UQuickTimeEventData* DataAsset = nullptr;
	AActor* Inflictor = nullptr;
};

/*
 * Blueprint function library for the quick time event system.
 */
UCLASS()
class METROXHUNTER_API UQuickTimeEventBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	 * Starts a quick time event as a latent function and outputs the result as execution pins.
	 * Designed for blueprint usage.
	 * 
	 * @param WorldContext	Context object to get the world from
	 * @param LatentInfo	Info of the latent action
	 * @param OutputPins	Output execution pins to trigger
	 * @param Component		Component to start the quick time event
	 * @param DataAsset		Data asset of the quick time event
	 * @param Inflictor		(optional) Actor causing the event
	 */
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent", meta = ( WorldContext = "WorldContext", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "OutputPins" ) )
	static void LatentQuickTimeEvent(
		UObject* WorldContext,
		FLatentActionInfo LatentInfo,
		EQuickTimeEventOutputPins& OutputPins,
		UQuickTimeEventComponent* Component,
		UQuickTimeEventData* DataAsset,
		AActor* Inflictor
	);
};
