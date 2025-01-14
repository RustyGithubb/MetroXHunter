/*
 * Implemented by Corentin Paya
 */

#include "Character/MetroPlayerCharacter.h"
#include "Reload/ReloadComponent.h"
#include "Inventory/InventoryComponent.h"

#include "CineCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

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

