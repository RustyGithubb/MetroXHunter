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

#include "Library/UtilityLibrary.h"

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
		EnhancedInputComponent->BindAction( MovementAction, ETriggerEvent::Triggered, this, &UCharacterControllerComponent::Move );
		EnhancedInputComponent->BindAction( MovementAction, ETriggerEvent::Completed, this, &UCharacterControllerComponent::OnMoveInputReleased );
		EnhancedInputComponent->BindAction( MovementAction, ETriggerEvent::Canceled, this, &UCharacterControllerComponent::OnMoveInputReleased );

		// Looking
		EnhancedInputComponent->BindAction( LookAction, ETriggerEvent::Triggered, this, &UCharacterControllerComponent::Look );

		// Toggle Run
		EnhancedInputComponent->BindAction( RunAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::ToggleRun );

		// Aim
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::ToggleAim );
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Completed, this, &UCharacterControllerComponent::ToggleAim );
	}

	SetupDefaultValues();
}

void UCharacterControllerComponent::SetupDefaultValues()
{
	MouseSensitivity = PlayerMovementData->DefaultMouseSensitivity;
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
	bCanRun = !bShouldFreeze;
}

FVector2D UCharacterControllerComponent::GetInputDirection()
{
	return Player->InputDirection;
}

void UCharacterControllerComponent::Move( const FInputActionValue& Value )
{
	if ( !bCanMove ) return;

	Player->GetCharacterMovement()->bUseControllerDesiredRotation = true;

	// Get input action value
	Player->InputDirection = Value.Get<FVector2D>();
	float CurveValue = PlayerMovementData->MovementCurve->GetFloatValue( Player->InputDirection.Length() );

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
	switch ( CurrentMovementState )
	{
		case EMovementState::Walk:
		{
			Player->ShootingImprecisionValue = 600.0f * Value.GetMagnitude();
			break;
		}
		case EMovementState::Run:
		{
			Player->ShootingImprecisionValue = 1000.0f * Value.GetMagnitude();
			break;
		}
	}
}

void UCharacterControllerComponent::ToggleRun()
{
	if ( Player->bIsAiming || !bCanRun ) return;

	switch ( CurrentMovementState )
	{
	case EMovementState::Walk:
	{
		CurrentMovementState = EMovementState::Run;
		Player->GetCharacterMovement()->MaxWalkSpeed = PlayerMovementData->DefaultRunSpeed;
		Player->UpdateTargetArmLength( PlayerMovementData->RunTargetArmLength );
		break;
	}
	case EMovementState::Run:
	{
		CurrentMovementState = EMovementState::Walk;
		Player->GetCharacterMovement()->MaxWalkSpeed = PlayerMovementData->DefaultWalkSpeed;
		Player->UpdateTargetArmLength( PlayerMovementData->WalkTargetArmLength );
		break;
	}
	}
}

void UCharacterControllerComponent::OnMoveInputReleased()
{
	Player->ShootingImprecisionValue = 0.0f;

	Player->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	Player->UpdateTargetArmLength( PlayerMovementData->IdleTargetArmLength );

	Player->InputDirection = FVector2D::Zero();
}

// TO DO: USE A SECOND CURVE DEPENDING ON DELTA TIME FOR THE ROTATION
void UCharacterControllerComponent::Look( const FInputActionValue& Value )
{
	if ( !bCanRotate ) return;

	// Get input action value
	Player->LookDirection = Value.Get<FVector2D>();
	float CurveValue = PlayerMovementData->RotationCurve->GetFloatValue( Player->LookDirection.Length() );

	// Add yaw and pitch input to controller
	Player->AddControllerYawInput( Player->LookDirection.X * MouseSensitivity.X * CurveValue );
	Player->AddControllerPitchInput( Player->LookDirection.Y * MouseSensitivity.Y * CurveValue );
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

		Player->bIsAiming = true;
		Player->StartAimAssist();
		Player->UpdateCameraFocal();
	}
	else
	{
		Player->bIsAiming = false;
		Player->StopAimAssist();

		if ( bWasRunning )
		{
			ToggleRun();
		}

		Player->ResetCameraFocal();
		Player->ResetCameraFocusPoint();

		// STOP AIM ASSIST
		// RESTORE RUNNING (if was running)
	}

}

