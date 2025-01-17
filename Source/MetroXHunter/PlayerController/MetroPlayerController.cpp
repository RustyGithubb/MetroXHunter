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

	verify( IsValid( InputSystem ) );
	verifyf(
		IsValid( NewMappingContext ),
		TEXT( "MetroPlayerController: Trying to set the input mapping context to an invalid asset" )
	);

	// Remove the latest switched mapping context
	if ( LastOverriddenMappingContext != nullptr )
	{
		InputSystem->RemoveMappingContext( LastOverriddenMappingContext );
	}

	// Remove default mapping contexts
	if ( bAreDefaultMappingContextsActive )
	{
		for ( const TSoftObjectPtr<UInputMappingContext>& SoftMappingContext : DefaultMappingContexts )
		{
			const UInputMappingContext* MappingContext = SoftMappingContext.LoadSynchronous();

			verifyf(
				IsValid( MappingContext ),
				TEXT( "MetroPlayerController: Trying to remove a default input mapping context from an invalid asset" )
			);

			InputSystem->RemoveMappingContext( MappingContext );
		}
		bAreDefaultMappingContextsActive = false;
	}

	// Switch to the new mapping context
	InputSystem->AddMappingContext( NewMappingContext, 0 );
	LastOverriddenMappingContext = NewMappingContext;
}

void AMetroPlayerController::ResetInputMappingContext_Implementation()
{
	if ( bAreDefaultMappingContextsActive ) return;

	// Ensure player controller is initialized before using it.
	if ( !HasActorBegunPlay() )
	{
		DispatchBeginPlay();
	}

	verify( IsValid( InputSystem ) );

	// Remove the latest switched mapping context
	if ( LastOverriddenMappingContext != nullptr )
	{
		InputSystem->RemoveMappingContext( LastOverriddenMappingContext );
	}
	LastOverriddenMappingContext = nullptr;

	// Add default mapping contexts
	for ( const TSoftObjectPtr<UInputMappingContext>& SoftMappingContext : DefaultMappingContexts )
	{
		const UInputMappingContext* MappingContext = SoftMappingContext.LoadSynchronous();

		verifyf(
			IsValid( MappingContext ),
			TEXT( "MetroPlayerController: Trying to add a default input mapping context from an invalid asset" )
		);

		InputSystem->AddMappingContext( MappingContext, 0 );
	}
	bAreDefaultMappingContextsActive = true;
}

void AMetroPlayerController::RevertInputMappingContext_Implementation( UInputMappingContext* MappingContext )
{
	if ( LastOverriddenMappingContext == MappingContext )
	{
		ResetInputMappingContext_Implementation();
	}
}

void AMetroPlayerController::TickDebug_Implementation( float DeltaTime, FString& OutDebugText )
{
	FString DefaultMappingContextsString = "";
	for ( const TSoftObjectPtr<UInputMappingContext>& SoftMappingContext : DefaultMappingContexts )
	{
		DefaultMappingContextsString += "- " + GetNameSafe( SoftMappingContext.Get() ) + "\n";
	}

	// Constructs formating arguments
	FStringFormatNamedArguments Args {};
	Args.Add( "bAreDefaultMappingContextsActive", bAreDefaultMappingContextsActive ? "true" : "false" );
	Args.Add( "LastOverriddenMappingContext", GetNameSafe( LastOverriddenMappingContext ) );
	Args.Add( "DefaultMappingContexts", DefaultMappingContextsString );
	Args.Add( "DefaultMappingContextsCount", DefaultMappingContexts.Num() );

	// Formats debug string
	constexpr auto Format = TEXT(
		"[MetroPlayerController]\n"
		"bAreDefaultMappingContextsActive: {bAreDefaultMappingContextsActive}\n"
		"LastOverriddenMappingContext: {LastOverriddenMappingContext}\n"
		"DefaultMappingContexts[{DefaultMappingContextsCount}]\n"
		"{DefaultMappingContexts}\n"
	);
	OutDebugText = FString::Format( Format, Args );
}
