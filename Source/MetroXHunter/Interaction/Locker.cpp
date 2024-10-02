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

#include "UtilityLibrary.h"
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

	BindInputs();
}

void ALocker::OnCancelInteraction()
{
	UnBindInputs();

	RemoveSkillCheckWidget();
	Widget->SetVisibility( true );

	ResetCameraTarget();
	UUtilityLibrary::PrintMessage( TEXT( "ON CANCEL INTERACTION" ) );
	bIsSkillCheckActive = false;
}

void ALocker::EndSkillCheck( bool bShouldReward )
{
	OnCancelInteraction();
	RemoveInteractionComponent();

	if ( bShouldReward )
	{
		SpawnLootItem();
	}
}

void ALocker::SpawnLootItem()
{
	if ( !ItemToSpawn ) return;

	FActorSpawnParameters SpawnInfo {};

	ABasePickUp* PickUp = GetWorld()->SpawnActor<ABasePickUp>(
		ItemToSpawn,
		ItemSpawnPoint->GetComponentTransform(),
		SpawnInfo
	);

	PickUp->Amount = ItemAmount;
}

void ALocker::BindInputs()
{
	verify( InteractAction != nullptr );
	verify( CancelInteractAction != nullptr );

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction(
			InteractAction.LoadSynchronous(), ETriggerEvent::Started,
			this, &ALocker::OnSkillCheckAttempt
		);

		// Cancel Interaction
		//EnhancedInputComponent->BindAction(
		//	CancelInteractAction.LoadSynchronous(), ETriggerEvent::Started,
		//	this, &ALocker::OnCancelInteraction
		//);
	}
}

void ALocker::UnBindInputs()
{
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		EnhancedInputComponent->ClearActionBindings();
	}
}