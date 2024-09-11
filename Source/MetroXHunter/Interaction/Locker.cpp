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

	// Smooth camera transition from the player to the Locker's camera
	PlayerController->SetViewTargetWithBlend(
		this,
		BlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		BlendExp
	);

	// Should refactor the inputs so we still see the Visualizer
	PlayerController->GetPawn()->DisableInput( PlayerController );
	PlayerController->DisableInput( PlayerController );

	bIsSkillCheckActive = true;
	SetActorTickEnabled( true );

	BindToInputs();
}

void ALocker::OnCancelInteraction()
{
	UnbindInputs();
	RemoveSkillCheckWidget();

	// Smooth camera transition from the Locker's camera to the Player
	PlayerController->SetViewTargetWithBlend(
		PlayerController->GetPawn(),
		BlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		BlendExp
	);

	PlayerController->GetPawn()->EnableInput( PlayerController );
	PlayerController->EnableInput( PlayerController );
}

void ALocker::EndSkillCheck()
{
	OnCancelInteraction();
	PlayerInteractionComponent->RemoveNearInteractable( InteractableComponent );

	InteractableComponent->DestroyComponent();
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
