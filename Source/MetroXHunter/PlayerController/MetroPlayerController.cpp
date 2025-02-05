/*
 * Implemented by Corentin Paya and Arthur Cathelain (arkaht)
 */

#include "PlayerController/MetroPlayerController.h"
#include "Gun/GunControllerComponent.h"
#include "Character/CharacterControllerComponent.h"
#include "Character/MetroPlayerCharacter.h"
#include "Character/PlayerMovementData.h"

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

	if ( IsValid( GunController ) )
	{
		GunController->SetupInputComponent( PlayerCharacter, InputComponent );
	}

	if ( IsValid( CharacterController ) )
	{
		CharacterController->SetupInputComponent( PlayerCharacter, InputComponent );

		PlayerCharacter->RunMaxSpeed = CharacterController->PlayerMovementData->DefaultRunSpeed;
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
			verifyf(
				!SoftMappingContext.IsNull(),
				TEXT( "MetroPlayerController: Trying to remove a default input mapping context from an invalid asset" )
			);

			const UInputMappingContext* MappingContext = SoftMappingContext.LoadSynchronous();
			InputSystem->RemoveMappingContext( MappingContext );
		}
		bAreDefaultMappingContextsActive = false;
	}

	// Switch to the new mapping context
	InputSystem->AddMappingContext( NewMappingContext, 1 );
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
		verifyf(
			!SoftMappingContext.IsNull(),
			TEXT( "MetroPlayerController: Trying to add a default input mapping context from an invalid asset" )
		);

		const UInputMappingContext* MappingContext = SoftMappingContext.LoadSynchronous();
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
		DefaultMappingContextsString += "- " + GetNameSafe( SoftMappingContext.Get() )
			+ "=" + FString::FromInt( InputSystem->HasMappingContext( SoftMappingContext.Get() ) ) + "\n";
	}

	// Constructs formating arguments
	FStringFormatNamedArguments Args {};
	Args.Add( "InputMode", InputModeDebugDisplayName );
	Args.Add( "bAreDefaultMappingContextsActive", bAreDefaultMappingContextsActive ? "true" : "false" );
	Args.Add( "LastOverriddenMappingContext", GetNameSafe( LastOverriddenMappingContext ) );
	Args.Add( "bLastOverriddenMappingContextActive", InputSystem->HasMappingContext( LastOverriddenMappingContext ) );
	Args.Add( "DefaultMappingContexts", DefaultMappingContextsString );
	Args.Add( "DefaultMappingContextsCount", DefaultMappingContexts.Num() );

	// Formats debug string
	constexpr auto Format = TEXT(
		"[MetroPlayerController]\n"
		"InputMode: {InputMode}\n"
		"bAreDefaultMappingContextsActive: {bAreDefaultMappingContextsActive}\n"
		"LastOverriddenMappingContext: {LastOverriddenMappingContext}={bLastOverriddenMappingContextActive}\n"
		"DefaultMappingContexts[{DefaultMappingContextsCount}]\n"
		"{DefaultMappingContexts}\n"
	);
	OutDebugText = FString::Format( Format, Args );
}

void AMetroPlayerController::SetInputMode( const FInputModeDataBase& InData )
{
	Super::SetInputMode( InData );

	InputModeDebugDisplayName = InData.GetDebugDisplayName();
}

void AMetroPlayerController::ToggleFreezePlayer_Implementation( bool bShouldFreeze )
{
	PlayerCharacter->bIsUnderAction = bShouldFreeze;
	CharacterController->ToggleFreezeMovement(bShouldFreeze);
	CharacterController->ToggleFreezeRotation(bShouldFreeze);
}
