/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "QuickTimeEvent/QuickTimeEventBlueprintLibrary.h"
#include "QuickTimeEvent/QuickTimeEventComponent.h"
#include "QuickTimeEvent/QuickTimeEventData.h"

void FLatentQuickTimeEvent::UpdateOperation( FLatentResponse& Response )
{
	// Start the quick time event
	if ( !bIsInitialized )
	{
		bIsInitialized = true;

		Component->StartEvent( DataAsset );
		return;
	}

	// Wait for the event to stop
	if ( Component->IsEventRunning() ) return;

	// Look for the result of the event
	switch ( Component->GetEventResult() )
	{
		case EQuickTimeEventResult::Succeed:
		{
			OutputPins = EQuickTimeEventOutputPins::OnSucceed;
			break;
		}
		case EQuickTimeEventResult::Failed:
		{
			OutputPins = EQuickTimeEventOutputPins::OnFailed;
			break;
		}
	}

	// Trigger the corresponding output pin and terminate
	Response.FinishAndTriggerIf( 
		true, 
		LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget 
	);
}

void UQuickTimeEventBlueprintLibrary::LatentQuickTimeEvent(
	UObject* WorldContext,
	FLatentActionInfo LatentInfo,
	EQuickTimeEventOutputPins& OutputPins,
	UQuickTimeEventComponent* Component,
	UQuickTimeEventData* DataAsset
)
{
	// Get world from context object (asserting)
	UWorld* World = GEngine->GetWorldFromContextObjectChecked( WorldContext );

	// Get latent action manager from the world
	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	// Try to retrieve an already existing action
	auto ExistingAction = LatentActionManager.FindExistingAction<FLatentQuickTimeEvent>(
		LatentInfo.CallbackTarget, LatentInfo.UUID
	);
	if ( ExistingAction != nullptr ) return;

	// Create a new action
	auto Action = new FLatentQuickTimeEvent( LatentInfo, OutputPins, Component, DataAsset );
	LatentActionManager.AddNewAction( LatentInfo.CallbackTarget, LatentInfo.UUID, Action );
}