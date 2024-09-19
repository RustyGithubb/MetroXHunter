/*
 * Implemented by Corentin Paya
 */

#include "Interaction/Locker.h"
#include "Interaction/InteractionComponent.h"
#include "Interaction/InteractableComponent.h"
#include "Interaction/BasePickUp.h"

#include "Components/SceneComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ALocker::ALocker()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled( false );

	ItemSpawnPoint = CreateDefaultSubobject<USceneComponent>( TEXT( "SceneComponent" ) );
	ItemSpawnPoint->SetupAttachment( RootComponent );
}

void ALocker::Interact()
{
	ShowSkillCheckWidget();
	Widget->SetVisibility( false );

	SwitchCameraTarget();

	bIsSkillCheckActive = true;
	SetActorTickEnabled( true );

	BindToInputs();
}

void ALocker::OnCancelInteraction()
{
	UnbindInputs();

	RemoveSkillCheckWidget();
	Widget->SetVisibility( true );

	ResetCameraTarget();
	bIsSkillCheckActive = false;
}

void ALocker::EndSkillCheck()
{
	OnCancelInteraction();
	SpawnLootItem();
	RemoveInteractionComponent();
}

void ALocker::SpawnLootItem()
{
	FActorSpawnParameters SpawnInfo {};

	ABasePickUp* PickUp = GetWorld()->SpawnActor<ABasePickUp>(
		ItemToSpawn,
		ItemSpawnPoint->GetComponentTransform(),
		SpawnInfo
	);

	PickUp->Amount = ItemAmount;
}

void ALocker::BindToInputs()
{
	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction(
			PlayerInteractionComponent->InteractAction, ETriggerEvent::Started, this,
			&ALocker::OnSkillCheckAttempt
		);

		// Cancel Interaction
		EnhancedInputComponent->BindAction(
			PlayerInteractionComponent->CancelInteractAction, ETriggerEvent::Started, this,
			&ALocker::OnCancelInteraction
		);
	}
}

void ALocker::UnbindInputs()
{
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		EnhancedInputComponent->ClearActionBindings();
	}
}
