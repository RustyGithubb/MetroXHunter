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
	if ( bIsGameEnded ) return;

	ShowSkillCheckWidget();
	Widget->SetVisibility( false );

	SwitchCameraTarget();

	bIsSkillCheckActive = true;
	SetActorTickEnabled( true );

	BindInputs();
}

void ALocker::EndSkillCheck( bool bShouldReward )
{
	bIsGameEnded = true;
	bIsSkillCheckActive = false;

	UnBindInputs();

	ResetCameraTarget();

	if ( bShouldReward )
	{
		SpawnLootItem();
	}

	RemoveInteractionComponent();
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

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction(
			InteractAction.LoadSynchronous(), ETriggerEvent::Started,
			this, &ALocker::OnSkillCheckAttempt
		);
	}
}

void ALocker::UnBindInputs()
{
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( InputComponent ) )
	{
		EnhancedInputComponent->ClearActionBindings();
	}
}