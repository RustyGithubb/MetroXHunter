#include "Interaction/Locker.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "MXHUtilityLibrary.h"

ALocker::ALocker()
{
	PrimaryActorTick.bCanEverTick = true;

	SetActorTickEnabled( false );
}

void ALocker::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void ALocker::Interact()
{
	ShowSkillCheckWidget();

	PlayerController->SetViewTargetWithBlend(
		this,
		BlendTime,
		EViewTargetBlendFunction::VTBlend_EaseInOut,
		BlendExp
	);

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
	UInputComponent* PlayerInputComponent = InputComponent;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
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
	UInputComponent* PlayerInputComponent = InputComponent;

	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
	{
		EnhancedInputComponent->ClearActionBindings();
	}
}
