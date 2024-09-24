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
}

void AMetroPlayerController::SetInputMappingContext_Implementation( UInputMappingContext* MappingContext )
{
	InputSystem->ClearAllMappings();

	InputSystem->AddMappingContext( MappingContext, 1 );
}

void AMetroPlayerController::ResetInputMappingContext_Implementation()
{
	SetInputMappingContext_Implementation( DefaultMappingContext.LoadSynchronous() );
}
