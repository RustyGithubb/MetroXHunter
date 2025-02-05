/*
 * Implemented by Corentin Paya
 */

#include "Character/CharacterControllerComponent.h"
#include "Character/MetroPlayerCharacter.h"
#include "Character/PlayerMovementData.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

UCharacterControllerComponent::UCharacterControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterControllerComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UCharacterControllerComponent::SetupInputComponent( AMetroPlayerCharacter* InPlayer, TObjectPtr<class UInputComponent> InputComponent )
{
	Player = InPlayer;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		// Moving
		EnhancedInputComponent->BindAction( MovementAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::OnMoveInputStart );
		EnhancedInputComponent->BindAction( MovementAction, ETriggerEvent::Triggered, this, &UCharacterControllerComponent::Move );
		EnhancedInputComponent->BindAction( MovementAction, ETriggerEvent::Completed, this, &UCharacterControllerComponent::OnMoveInputReleased );
		EnhancedInputComponent->BindAction( MovementAction, ETriggerEvent::Canceled, this, &UCharacterControllerComponent::OnMoveInputReleased );

		// Looking
		EnhancedInputComponent->BindAction( LookAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::OnLookInputStart );
		EnhancedInputComponent->BindAction( LookAction, ETriggerEvent::Triggered, this, &UCharacterControllerComponent::Look );

		// Toggle Run
		EnhancedInputComponent->BindAction( RunAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::ToggleRun );

		// Aim
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::ToggleAim );
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Completed, this, &UCharacterControllerComponent::ToggleAim );
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Canceled, this, &UCharacterControllerComponent::ToggleAim );

		// Heal
		EnhancedInputComponent->BindAction( HealAction, ETriggerEvent::Completed, Player, &AMetroPlayerCharacter::UseSyringe );

		// Stomp
		EnhancedInputComponent->BindAction( StompAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::StompKick );
	}

	SetupDefaultValues();
}

void UCharacterControllerComponent::SetupDefaultValues()
{
	CameraSensitivity = PlayerMovementData->DefaultMouseSensitivity;
	Player->GetCharacterMovement()->MaxWalkSpeed = PlayerMovementData->DefaultWalkSpeed;
}

void UCharacterControllerComponent::ToggleFreezeMovement( bool bShouldFreeze )
{
	bCanMove = !bShouldFreeze;
}

void UCharacterControllerComponent::ToggleFreezeRotation( bool bShouldFreeze )
{
	bCanRotate = !bShouldFreeze;
}

void UCharacterControllerComponent::ToggleFreezeRun( bool bShouldFreeze )
{
	if ( bShouldFreeze && bIsRunning )
	{
		ToggleRun();
	}

	bCanRun = !bShouldFreeze;
}

FVector2D UCharacterControllerComponent::GetInputDirection()
{
	return Player->InputDirection;
}

void UCharacterControllerComponent::OnMoveInputStart( const FInputActionValue& Value )
{
	Player->UpdateCrossHairOnMovement( CrossHairTranslationStrength );

	HandleToggleRun();
}

void UCharacterControllerComponent::OnMoveInputReleased()
{
	bIsMoving = false;
	Player->UpdateCrossHairOnMovement( 0.0f );
	Player->ShootingImprecisionValue = 0.0f;

	Player->GetCharacterMovement()->bUseControllerDesiredRotation = false;

	Player->UpdateTargetArmLength( PlayerMovementData->IdleTargetArmLength );

	Player->InputDirection = FVector2D::Zero();

	CurrentMovementState = EMovementState::Idle;
	OnMovementStateUpdate.Broadcast( CurrentMovementState );
}

void UCharacterControllerComponent::Move( const FInputActionValue& Value )
{
	if ( !bCanMove )
	{
		bIsMoving = false;
		return;
	}

	bIsMoving = true;
	Player->GetCharacterMovement()->bUseControllerDesiredRotation = true;

	// Get input action value
	Player->InputDirection = Value.Get<FVector2D>();
	float CurveValue = PlayerMovementData->MovementInputLengthCurve->GetFloatValue( Player->InputDirection.Length() );

	// Calculate Movement Direction
	FVector MakeVector( Player->InputDirection.Y, Player->InputDirection.X, 0 );
	FVector MovementDirection = UKismetMathLibrary::TransformDirection(
		Player->GetActorTransform(),
		MakeVector * CurveValue
	);

	// Check if the player is moving backward
	float ScalarMovement = Player->InputDirection.Y < 0 ? 0.75f : 1.0f;
	Player->AddMovementInput( MovementDirection * ScalarMovement );

	// Update Shooting Imprecision value
	// We don't update when running as we can't shoot while running
	if ( CurrentMovementState == EMovementState::Walk )
	{
		Player->ShootingImprecisionValue = 150 * Player->InputDirection.Length();
	}
}

void UCharacterControllerComponent::OnLookInputStart( const FInputActionValue& Value )
{
	RotationInputTimer = GetWorld()->GetTimeSeconds();;
}

void UCharacterControllerComponent::Look( const FInputActionValue& Value )
{
	if ( !bCanRotate ) return;

	float Timer = GetWorld()->GetTimeSeconds() - RotationInputTimer;

	// Get input action value
	Player->LookDirection = Value.Get<FVector2D>();
	float CurveIntensityValue = PlayerMovementData->RotationInputLengthCurve->GetFloatValue( Player->LookDirection.Length() );
	float CurveDurationValue = PlayerMovementData->RotationDurationCurve->GetFloatValue( Timer );

	// TODO: ADD AIM ASSIST DECELERATION RATE (replace mouse sensitivity by mouse sensitivity * deceleration rate)

	// Add yaw and pitch input to controller
	Player->AddControllerYawInput( Player->LookDirection.X * CameraSensitivity.X * CurveIntensityValue * CurveDurationValue );
	Player->AddControllerPitchInput( Player->LookDirection.Y * CameraSensitivity.Y * CurveIntensityValue * CurveDurationValue );
}

void UCharacterControllerComponent::StompKick()
{
	if ( Player->bIsUnderAction || Player->bIsAiming ) return;

	float InitialVelocity = UKismetMathLibrary::VSizeXY( Player->GetVelocity() );

	FVector MakeVector( Player->InputDirection.Y, Player->InputDirection.X, 0 );
	FVector Direction = UKismetMathLibrary::TransformDirection(
		Player->GetActorTransform(),
		MakeVector
	);

	ToggleFreezeMovement( true );
	ToggleFreezeRotation( true );
	Player->OnStompKick( InitialVelocity, Direction );
}

void UCharacterControllerComponent::ToggleRun()
{
	if ( Player->bIsAiming || !bCanRun ) return;

	bIsRunning = !bIsRunning;

	if ( !bIsMoving ) return;

	HandleToggleRun();
}

void UCharacterControllerComponent::HandleToggleRun()
{
	if ( bIsRunning )
	{
		CurrentMovementState = EMovementState::Run;
		Player->GetCharacterMovement()->MaxWalkSpeed = PlayerMovementData->DefaultRunSpeed;
		Player->UpdateTargetArmLength( PlayerMovementData->RunTargetArmLength );
	}
	else
	{
		CurrentMovementState = EMovementState::Walk;
		Player->GetCharacterMovement()->MaxWalkSpeed = PlayerMovementData->DefaultWalkSpeed;
		Player->UpdateTargetArmLength( PlayerMovementData->WalkTargetArmLength );
	}

	OnMovementStateUpdate.Broadcast( CurrentMovementState );
}

void UCharacterControllerComponent::ToggleAim( const FInputActionValue& Value )
{
	if ( !Player->bIsWeaponEquipped ) return;

	if ( Value.Get<bool>() )
	{
		if ( CurrentMovementState == EMovementState::Run )
		{
			bWasRunning = true;
			ToggleRun();
		}
		else
		{
			bWasRunning = false;
		}

		Player->StartAiming();
	}
	else
	{
		Player->StopAiming();

		if ( bWasRunning )
		{
			ToggleRun();
		}
	}
}

