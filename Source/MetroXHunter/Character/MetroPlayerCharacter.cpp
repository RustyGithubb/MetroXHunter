/*
 * Implemented by Corentin Paya
 */

#include "Character/MetroPlayerCharacter.h"
#include "Reload/ReloadComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Interaction/InteractionComponent.h"
#include "Health/HealthComponent.h"
#include "Electricity/ElectrocutableComponent.h"
#include "Checkpoint/SaveLoadComponent.h"
#include "AI/AITargetComponent.h"
#include "QuickTimeEvent/QuickTimeEventComponent.h"
#include "DualSenseControllerComponent.h"

#include "CineCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Library/UtilityLibrary.h"

AMetroPlayerCharacter::AMetroPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>( TEXT( "SpringArm" ) );
	SpringArmComponent->SetupAttachment( RootComponent );

	// TODO: CHECK IF THE PIVOT CAMERA IS NECESSARY !
	PivotCamera = CreateDefaultSubobject<USceneComponent>( TEXT( "PivotCamera" ) );
	PivotCamera->SetupAttachment( SpringArmComponent );

	CineCameraComponent = CreateDefaultSubobject<UCineCameraComponent>( TEXT( "CineCamera" ) );
	CineCameraComponent->SetupAttachment( PivotCamera );

	ReloadComponent = CreateDefaultSubobject<UReloadComponent>( TEXT( "Reload" ) );
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>( TEXT( "Inventory" ) );
	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "Health" ) );
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>( TEXT( "Interaction" ) );
	SaveLoadComponent = CreateDefaultSubobject<USaveLoadComponent>( TEXT( "SaveLoad" ) );
	ElectrocutableComponent = CreateDefaultSubobject<UElectrocutableComponent>( TEXT( "Electrocutable" ) );
	AITargetComponent = CreateDefaultSubobject<UAITargetComponent>( TEXT( "AITarget" ) );
	QTEComponent = CreateDefaultSubobject<UQuickTimeEventComponent>( TEXT( "QTE" ) );

	DualSenseComponent = CreateDefaultSubobject<UDualSenseControllerComponent>( TEXT( "DualSenseControllerComponent" ) );
}

void AMetroPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMetroPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( bIsAiming )
	{
		UpdateCameraFocusPoint();
	}
}

void AMetroPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMetroPlayerCharacter::StartAiming()
{
	bIsAiming = true;
	StartAimAssist();
	UpdateCameraFocal();

	OnAim.Broadcast( bIsAiming );
}

void AMetroPlayerCharacter::StopAiming_Implementation()
{
	bIsAiming = false;
	StopAimAssist();
	ResetCameraFocal();
	ResetCameraFocusPoint();

	OnAim.Broadcast( bIsAiming );
}
