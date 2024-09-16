#include "Reload/ReloadComponent.h"
#include "Reload/ReloadData.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

UReloadComponent::UReloadComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UReloadComponent::BeginPlay()
{
	Super::BeginPlay();

	GetReferences();
	SetupPlayerInputComponent();
}

void UReloadComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UReloadComponent::SetupPlayerInputComponent()
{
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
	{
		// Interaction
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this,&UReloadComponent::StartReloadSequence);
	}
}

void UReloadComponent::StartReloadSequence()
{
}

void UReloadComponent::UpdateCurrentReloadState()
{

}

void UReloadComponent::UpdateAmmoCount()
{

}

void UReloadComponent::GetReferences()
{

}

void UReloadComponent::GetCurrentReloadState()
{

}

void UReloadComponent::GetAmmoData()
{

}

void UReloadComponent::GetNormalizedReloadTimings()
{

}

void UReloadComponent::GetNormalizedReloadElapsedTime()
{

}

void UReloadComponent::ComputeReloadAmmoCount()
{

}

void UReloadComponent::OnReloadInput()
{

}
