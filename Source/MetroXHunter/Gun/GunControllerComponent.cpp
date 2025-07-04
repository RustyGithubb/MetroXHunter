/*
 * Implemented by Corentin Paya
 */

#include "Gun/GunControllerComponent.h"
#include "Gun/Gun.h"
#include "Character/MetroPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

#include "Library/UtilityLibrary.h"

UGunControllerComponent::UGunControllerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGunControllerComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UGunControllerComponent::SetupInputComponent( AMetroPlayerCharacter* InPlayer, TObjectPtr<class UInputComponent> InputComponent )
{
	Player = InPlayer;
	Springfield = Cast<AGun>(Player->BaseGun);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent );

	if ( EnhancedInputComponent )
	{
		// Shooting
		EnhancedInputComponent->BindAction( ShootAction, ETriggerEvent::Triggered, this, &UGunControllerComponent::OnShootActionTriggered );

		// Switch weapon
		EnhancedInputComponent->BindAction( SwitchGunModeAction, ETriggerEvent::Started, Springfield, &AGun::SwitchWeapon );
	}
}

void UGunControllerComponent::OnShootActionPressed()
{
	bIsTriggerInputDown = true;
	Springfield->bShouldUpdateDualSense = false;

	switch ( Springfield->GunMode )
	{
		case EGunMode::Bullet:
		{
			// Get adjusted Aim Assist impact point
			FVector AdjustedImpact = Player->GetAdjustedImpactPoint();
			Springfield->TriggerShootAbility( AdjustedImpact );
			break;
		}
		case EGunMode::Lightning:
		{
			Springfield->OnLightningStart();
			break;
		}
	}
}

void UGunControllerComponent::OnShootActionTriggered( const FInputActionValue& Value )
{
	// Released shoot if not aiming or under action
	if ( ( Player->bIsUnderAction || !Player->bIsAiming ) && bIsTriggerInputDown )
	{
		OnShootActionCompleted();
		return;
	}

	if ( !Player->bIsAiming || Player->bIsUnderAction || !Player->bCanAim ) return;

	// On Shoot Down
	if ( !bIsTriggerInputDown )
	{
		if ( Value.Get<float>() > 0.9f )							
		{
			OnShootActionPressed();
		}

		return;
	}

	// On Shoot Released
	if( bIsTriggerInputDown && Value.Get<float>() < 0.3f ) 			
	{
		bIsTriggerInputDown = false;
		Springfield->bShouldUpdateDualSense = true;

		OnShootActionCompleted();
		return;
	}
	
	// On Shoot Triggered
	if (bIsTriggerInputDown
		&& Value.Get<float>() > 0.9f
		&& Springfield->GunMode == EGunMode::Lightning)
	{
		Springfield->OnLightningAbility( Value.Get<float>() );	

		return;
	}

	// Cancel Lightning if not in lightning mode
	if (bIsTriggerInputDown
		&& Value.Get<float>() > 0.9f
		&& Springfield->GunMode == EGunMode::Bullet )
	{
		switch ( Springfield->LightningState )
		{
		case ELightningState::Active:
		{
			Springfield->OnLightningEnd();
			break;
		}

		case ELightningState::Charging:
		{
			Springfield->OnChargeStop();
			break;
		}
		}
	}
}

void UGunControllerComponent::OnShootActionCompleted()
{
	switch ( Springfield->LightningState )
	{
	case ELightningState::Active:
	{
		Springfield->OnLightningEnd();
		break;
	}

	case ELightningState::Charging:
	{
		Springfield->OnChargeStop();
		break;
	}
	}
}
