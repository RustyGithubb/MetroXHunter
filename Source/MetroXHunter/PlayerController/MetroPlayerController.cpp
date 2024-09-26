/*
 * Implemented by Corentin Paya and Arthur Cathelain (arkaht)
 */

#include "PlayerController/MetroPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void AMetroPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InputSystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	ResetInputMappingContext_Implementation();
}

void AMetroPlayerController::SetInputMappingContext_Implementation( UInputMappingContext* NewMappingContext )
{
	// Remove the latest switched mapping context
	if ( LastMappingContext != nullptr )
	{
		InputSystem->RemoveMappingContext( LastMappingContext );
	}

	// Switch to the new mapping context
	InputSystem->AddMappingContext( NewMappingContext, 0 );
	LastMappingContext = NewMappingContext;
}

void AMetroPlayerController::ResetInputMappingContext_Implementation()
{
	SetInputMappingContext_Implementation( DefaultMappingContext.LoadSynchronous() );
}
