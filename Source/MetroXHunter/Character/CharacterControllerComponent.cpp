/*
 * Implemented by Corentin Paya
 */

#include "Character/CharacterControllerComponent.h"
#include "Character/MetroPlayerCharacter.h"
#include "Character/PlayerMovementData.h"

#include <DualSenseFunctionLibrary.h>
#include "DualSenseControllerComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CameraAnimationSequence.h"
#include "CineCameraComponent.h"
#include "CommonInputSubsystem.h"
#include "GameFramework/CheatManagerDefines.h" // UE_WITH_CHEAT_MANAGER

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
	PlayerController = Cast<APlayerController>(GetOwner());

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
		EnhancedInputComponent->BindAction( LookAction, ETriggerEvent::Completed, this, &UCharacterControllerComponent::OnLookInputReleased );
		EnhancedInputComponent->BindAction( LookAction, ETriggerEvent::Canceled, this, &UCharacterControllerComponent::OnLookInputReleased );

		// Toggle Run
		EnhancedInputComponent->BindAction( RunAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::ToggleRun );
		EnhancedInputComponent->BindAction( RunAction, ETriggerEvent::Completed, this, &UCharacterControllerComponent::OnToggleRunInputReleased );

		// Aim
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Triggered, this, &UCharacterControllerComponent::ToggleAim );
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Completed, this, &UCharacterControllerComponent::ToggleAim );
		EnhancedInputComponent->BindAction( AimAction, ETriggerEvent::Canceled, this, &UCharacterControllerComponent::ToggleAim );

		// Heal
		EnhancedInputComponent->BindAction( HealAction, ETriggerEvent::Completed, Player, &AMetroPlayerCharacter::UseSyringe );

		// Stomp
		EnhancedInputComponent->BindAction( StompAction, ETriggerEvent::Started, this, &UCharacterControllerComponent::OnMeleeInput );
	}

	SetupDefaultValues();
}

void UCharacterControllerComponent::SetupDefaultValues()
{
	CameraSensitivity = PlayerMovementData->DefaultMouseSensitivity;
	Player->GetCharacterMovement()->MaxWalkSpeed = PlayerMovementData->DefaultWalkSpeed;

	Player->SpringArmComponent->CameraLagSpeed = PlayerMovementData->MovementCameraLag;
	Player->SpringArmComponent->CameraRotationLagSpeed = PlayerMovementData->MovementCameraRotationLag;
	Player->SpringArmComponent->SocketOffset = PlayerMovementData->DefaultTargetOffset;
	Player->SpringArmComponent->TargetArmLength = PlayerMovementData->IdleTargetArmLength;
}

void UCharacterControllerComponent::ToggleFreezeMovement( bool bShouldFreeze )
{
	bCanMove = !bShouldFreeze;

	if ( !bCanMove )
	{
		OnMoveInputReleased();
		HandleToggleRun();
	}
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
	if ( !bCanMove ) return;

	bIsMoving = true;

	if ( GetWorld()->GetTimeSeconds() - StopMovementTimer < PlayerMovementData->DelayStopStart
		&& bWasRunningBeforeStop )
	{
		ToggleRun();
	}

	HandleToggleRun();
}

void UCharacterControllerComponent::OnMoveInputReleased()
{
	if ( !bIsMoving ) return;

	StopMovementTimer = GetWorld()->GetTimeSeconds();
	bWasRunningBeforeStop = bIsRunning;

	Player->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	Player->UpdateTargetArmLength( PlayerMovementData->IdleTargetArmLength );

	Player->InputDirection = FVector2D::Zero();

	bIsMoving = false;
	bIsRunning = false;
	CurrentMovementState = EMovementState::Idle;

	OnMovementStateUpdate.Broadcast( CurrentMovementState );
	Player->PlayMovementCameraAnimation( PlayerMovementData->IdleCameraAnimation );
}

void UCharacterControllerComponent::Move( const FInputActionValue& Value )
{
	if ( !bCanMove )
	{
		bIsMoving = false;
		return;
	}

	bIsMoving = true;
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Get input action value
	FVector2D InputDirection = Value.Get<FVector2D>();
	Player->InputDirection = InputDirection;
	Player->InputDirection.Normalize();

	FRotator ControllerYawRotation { 0, PlayerController->GetControlRotation().Yaw, 0 };
	FVector RightVector = UKismetMathLibrary::GetRightVector( ControllerYawRotation );
	FVector ForwardVector = UKismetMathLibrary::GetForwardVector( ControllerYawRotation );

#ifdef UE_WITH_CHEAT_MANAGER
	// NOTE: When noclipping, we want to move forward using the camera and not the player.
	if ( Player->GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying )
	{
		RightVector = UKismetMathLibrary::GetRightVector( PlayerController->GetControlRotation() );
		ForwardVector = UKismetMathLibrary::GetForwardVector( PlayerController->GetControlRotation() );
	}
#endif

	FVector MovementDirection = ( RightVector * InputDirection.X + ForwardVector * InputDirection.Y );

	// Check if the player is moving backward
	float ScalarMovement = Player->InputDirection.Y < 0 ? 0.75f : 1.0f;
	float CurveValue = PlayerMovementData->MovementInputLengthCurve->GetFloatValue( InputDirection.Length() );

	Player->AddMovementInput( MovementDirection, ScalarMovement * CurveValue );
}

void UCharacterControllerComponent::OnLookInputStart( const FInputActionValue& Value )
{
	RotationInputTimer = GetWorld()->GetTimeSeconds();
}

void UCharacterControllerComponent::Look( const FInputActionValue& Value )
{
	if ( !bCanRotate ) return;

	float Timer = GetWorld()->GetTimeSeconds() - RotationInputTimer;

	if ( Player->bIsAiming && GetWorld()->GetTimeSeconds() - DotTimer > DotPrecision )
	{
		float DotProduct = FVector2D::DotProduct( LastInputDirection, Value.Get<FVector2D>().GetSafeNormal() );
		DotTimer = GetWorld()->GetTimeSeconds();

		LastInputDirection = Player->LookDirection.GetSafeNormal();

		if ( DotProduct < 0.9f )
		{
			Timer = Timer > 0.1f ? 0.1f : Timer;
		}
	}

	float DeltaTime = FMath::Min( GetWorld()->GetDeltaSeconds(), 0.1 );

	// Get input action value
	Player->LookDirection = Value.Get<FVector2D>();

	float CurveIntensityValue = 1.0f;
	float CurveDurationValue = 1.0f;
	float AimMultiplier = 1.0f;

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	const UCommonInputSubsystem* CommonInputSubsystem = LocalPlayer->GetSubsystem<UCommonInputSubsystem>();
	if ( LocalPlayer  && CommonInputSubsystem )
	{
		if ( Player->CurrentInputType != CommonInputSubsystem->GetCurrentInputType() )
		{
			Player->CurrentInputType = CommonInputSubsystem->GetCurrentInputType();
			Player->OnInputTypeUpdate.Broadcast( Player->CurrentInputType );
		}

		if ( Player->CurrentInputType == ECommonInputType::Gamepad )
		{
			CurveIntensityValue = PlayerMovementData->RotationInputLengthCurve->GetFloatValue( Player->LookDirection.Length() );

			if ( Player->bIsAiming )
			{
				CurveDurationValue = PlayerMovementData->RotationDurationCurve->GetFloatValue( Timer );
				AimMultiplier = 0.8f;
			}
		}
	}

	// Add yaw and pitch input to controller
	Player->AddControllerYawInput( Player->LookDirection.X * CameraSensitivity.X * AimMultiplier * Player->AimDecelerationRate * CurveIntensityValue * CurveDurationValue * DeltaTime );
	Player->AddControllerPitchInput( Player->LookDirection.Y * CameraSensitivity.Y * AimMultiplier * Player->AimDecelerationRate * CurveIntensityValue * CurveDurationValue * DeltaTime );
}

void UCharacterControllerComponent::OnLookInputReleased()
{
	Player->LookDirection = FVector2D( 0 );
}

void UCharacterControllerComponent::OnMeleeInput()
{
	if ( Player->bIsUnderAction || Player->bIsAiming || !Player->bIsWeaponEquipped ) return;

	Player->TimeSinceLastAction = GetWorld()->GetTimeSeconds();

	float InitialVelocity = UKismetMathLibrary::VSizeXY( Player->GetVelocity() );

	FVector MakeVector( Player->InputDirection.Y, Player->InputDirection.X, 0 );
	FVector Direction = UKismetMathLibrary::TransformDirection(
		Player->GetActorTransform(),
		MakeVector
	);

	ToggleFreezeMovement( true );
	Player->GetCharacterMovement()->StopActiveMovement();

	// Check Player Direction aim
	FVector ForwardVectorController = UKismetMathLibrary::GetForwardVector( Player->GetControlRotation() );
	FVector UpVectorPlayer = Player->GetActorUpVector();
	float DotProductValue = UKismetMathLibrary::Dot_VectorVector( UpVectorPlayer, ForwardVectorController );

	Player->OnMeleeAttack( InitialVelocity, DotProductValue );
}

void UCharacterControllerComponent::OnToggleRunInputReleased()
{
	if ( !bIsToggleRunMode && bIsRunning ) ToggleRun();
}

void UCharacterControllerComponent::ToggleRun()
{
#ifdef UE_WITH_CHEAT_MANAGER
	// NOTE: When noclipping, we want to cancel noclip by toggling run.
	UCharacterMovementComponent* CharacterMovementComponent = Player->GetCharacterMovement();
	if ( CharacterMovementComponent->MovementMode == EMovementMode::MOVE_Flying )
	{
		Player->ClientCheatWalk_Implementation();
	}
#endif

	if ( Player->bIsAiming || Player->bShouldStartAiming || !bCanRun ) return;

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
		Player->PlayMovementCameraAnimation( PlayerMovementData->RunCameraAnimation );
	}
	else
	{
		CurrentMovementState = EMovementState::Walk;
		Player->GetCharacterMovement()->MaxWalkSpeed = PlayerMovementData->DefaultWalkSpeed;
		Player->UpdateTargetArmLength( PlayerMovementData->WalkTargetArmLength );
		Player->PlayMovementCameraAnimation( PlayerMovementData->WalkCameraAnimation );
	}

	OnMovementStateUpdate.Broadcast( CurrentMovementState );
}

void UCharacterControllerComponent::ToggleAim( const FInputActionValue& Value )
{
	if ( !Player->bIsWeaponEquipped ) return;

	if ( Player->bIsUnderAction &&
		GetWorld()->GetTimeSeconds() - Player->TimeSinceLastAction <= Player->CancelActionTimer )
	{
		Player->CancelCurrentAction();
		Player->bIsUnderAction = false;
	}

	if ( Value.Get<bool>() && !Player->bIsUnderAction )
	{
		if ( Player->bShouldStartAiming ) return;

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
		Player->UpdateTargetArmOffset( PlayerMovementData->AimTargetOffset );
		Player->SpringArmComponent->CameraLagSpeed = PlayerMovementData->AimCameraLag;
		Player->SpringArmComponent->CameraRotationLagSpeed = PlayerMovementData->AimCameraRotationLag;
	}
	else
	{
		if ( !Player->bShouldStartAiming ) return;

		Player->StopAiming();
		Player->UpdateTargetArmOffset( PlayerMovementData->DefaultTargetOffset );
		Player->SpringArmComponent->CameraLagSpeed = PlayerMovementData->MovementCameraLag;
		Player->SpringArmComponent->CameraRotationLagSpeed = PlayerMovementData->MovementCameraRotationLag;

		if ( bWasRunning && bIsMoving )
		{
			ToggleRun();
		}

		Player->DualSenseComponent->SetRightTriggerEffect( UDualSenseFunctionLibrary::MakeOff() );
	}
}
