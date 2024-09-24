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
#include "InputMappingContext.h"

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
	RemoveSkillCheckWidget();
	Widget->SetVisibility( true );

	ResetCameraTarget();
	bIsSkillCheckActive = false;
}

void ALocker::EndSkillCheck()
{
	OnCancelInteraction();
	RemoveInteractionComponent();
	SpawnLootItem();
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
	verify( InteractAction != nullptr );
	verify( CancelInteractAction != nullptr );

	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction(
			InteractAction, ETriggerEvent::Started, this,
			&ALocker::OnSkillCheckAttempt
		);

		// Cancel Interaction
		EnhancedInputComponent->BindAction(
			CancelInteractAction, ETriggerEvent::Started, this,
			&ALocker::OnCancelInteraction
		);
	}
}