/*
 * Implemented by Corentin Paya
 */

#include "Interaction/Locker.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ALocker::ALocker()
{
	PrimaryActorTick.bCanEverTick = true;

	SetActorTickEnabled( false );
}

void ALocker::Interact()
{
	ShowSkillCheckWidget();

	SwitchCameraTarget();

	bIsSkillCheckActive = true;
	SetActorTickEnabled( true );

	BindToInputs();
}

void ALocker::OnCancelInteraction()
{
	UnbindInputs();
	RemoveSkillCheckWidget();

	ResetCameraTarget();
	bIsSkillCheckActive = false;
}

void ALocker::EndSkillCheck()
{
	OnCancelInteraction();
}

void ALocker::BindToInputs()
{
	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction(
			PlayerInteractionComponent->InteractAction, ETriggerEvent::Started, this,
			&ALocker::OnSkillCheckAttempt
		);

		// Cancel Interaction
		EnhancedInputComponent->BindAction(
			PlayerInteractionComponent->CancelInteractAction, ETriggerEvent::Started, this,
			&ALocker::OnCancelInteraction
		);
	}
}

void ALocker::UnbindInputs()
{
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		EnhancedInputComponent->ClearActionBindings();
	}
}
