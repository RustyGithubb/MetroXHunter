/*
 * Implemented by Corentin Paya and Arthur Cathelain (arkaht)
 */

#include "PlayerController/MetroPlayerController.h"
#include "Gun/GunControllerComponent.h"
#include "Character/CharacterControllerComponent.h"
#include "Character/MetroPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include <Kismet/GameplayStatics.h>

void AMetroPlayerController::BeginPlay()
{
	Super::BeginPlay();

	InputSystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	ResetInputMappingContext_Implementation();

	GetWorld()->OnWorldBeginPlay.AddUObject( this, &AMetroPlayerController::LateBeginPlay );
}

void AMetroPlayerController::LateBeginPlay()
{
	//	arkaht: Can't we use GetPawn() instead of GetPlayerCharacter here? If it doesn't work
	//	because GetPawn() returns nullptr at this time, maybe try in OnPossess?
	PlayerCharacter = Cast<AMetroPlayerCharacter>( UGameplayStatics::GetPlayerCharacter( GetWorld(), 0 ) );

	GunController = GetComponentByClass<UGunControllerComponent>();
	CharacterController = GetComponentByClass<UCharacterControllerComponent>();

	if ( GunController )
	{
		GunController->SetupInputComponent( PlayerCharacter, InputComponent );
	}

	if ( CharacterController )
	{
		CharacterController->SetupInputComponent( PlayerCharacter, InputComponent );
	}
}

void AMetroPlayerController::SetInputMappingContext_Implementation( UInputMappingContext* NewMappingContext )
{
	// Ensure player controller is initialized before using it.
	if ( !HasActorBegunPlay() )
	{
		DispatchBeginPlay();
	}

	verifyf(
		IsValid( InputSystem ),
		TEXT( "MetroPlayerController: InputSystem isn't valid yet, are you using it too early?" )
	);
	verifyf(
		IsValid( NewMappingContext ),
		TEXT( "MetroPlayerController: Trying to set the input mapping context to an invalid asset" )
	);

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

void AMetroPlayerController::RevertInputMappingContext_Implementation( UInputMappingContext* MappingContext )
{
	if ( LastMappingContext == MappingContext )
	{
		ResetInputMappingContext_Implementation();
	}
}
