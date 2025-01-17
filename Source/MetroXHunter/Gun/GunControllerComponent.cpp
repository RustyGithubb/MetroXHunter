/*
 * Implemented by Corentin Paya
 */

#include "Gun/GunControllerComponent.h"
#include "Gun/Gun.h"
#include "Character/MetroPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

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
		EnhancedInputComponent->BindAction( ShootAction, ETriggerEvent::Started, this, &UGunControllerComponent::OnShootActionPressed );
		EnhancedInputComponent->BindAction( ShootAction, ETriggerEvent::Triggered, this, &UGunControllerComponent::OnShootActionTriggered );
		EnhancedInputComponent->BindAction( ShootAction, ETriggerEvent::Completed, this, &UGunControllerComponent::OnShootActionCompleted );

		// Switch weapon
		EnhancedInputComponent->BindAction( SwitchGunModeAction, ETriggerEvent::Started, Springfield, &AGun::SwitchWeapon );
	}
}

void UGunControllerComponent::OnShootActionPressed()
{
	if ( !Player->bIsAiming ) return;

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
	if ( Springfield->GunMode != EGunMode::Lightning ) return;

	if ( Player->bIsAiming )
	{
		Springfield->OnLightningAbility( Value.Get<float>());
	}
	else
	{
		Springfield->OnLightningEnd();
	}
}

void UGunControllerComponent::OnShootActionCompleted()
{
	if ( Springfield->GunMode != EGunMode::Lightning ) return;

	Springfield->OnLightningEnd();
}
