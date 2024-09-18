/*
 * Implemented by BARRAU Benoit
 */

#include "Reload/ReloadComponent.h"
#include "Reload/ReloadData.h"
#include "HUD/MainHUD.h"
#include "Reload/IReloadSystemUpdate.h"
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
	UpdateAmmoCount(MaxMagazineAmmoCount);
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

void UReloadComponent::UpdateCurrentReloadState(EGunReloadState NewState)
{
	CurrentReloadState = NewState;

	if( HUD )
	{
		if ( IIReloadSystemUpdate* ReloadSystemInterface = Cast<IIReloadSystemUpdate>( HUD ) )
		{
			ReloadSystemInterface->UpdateReloadSystemState( CurrentReloadState );
		}
	}
}

EGunReloadState UReloadComponent::GetCurrentReloadState() const
{
	return CurrentReloadState;
}

void UReloadComponent::InitializeReloadData( UReloadData* NewReloadData )
{
	if ( NewReloadData )
	{
		ReloadDataAsset = NewReloadData;
		UE_LOG( LogTemp, Warning, TEXT( "Reload Data Asset initialized !" ));
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "Failed to initialize Reload Data Asset !" ));
	}
}

void UReloadComponent::GetNormalizedReloadTimings( float& OutPerfectReloadStartTime, float& OutActiveReloadStartTime, float& OutActiveReloadEndTime ) const
{
	if ( ReloadDataAsset && ReloadDataAsset->NormalReloadDuration > 0.0f )
	{
		OutPerfectReloadStartTime = ReloadDataAsset->PerfectReloadStartTime / ReloadDataAsset->NormalReloadDuration;
		OutActiveReloadStartTime = ReloadDataAsset->ActiveReloadStartTime / ReloadDataAsset->NormalReloadDuration;
		OutActiveReloadEndTime = ReloadDataAsset->ActiveReloadEndTime / ReloadDataAsset->NormalReloadDuration;
	}
	else
	{
		OutPerfectReloadStartTime = 0.0f;
		OutActiveReloadStartTime = 0.0f;
		OutActiveReloadEndTime = 0.0f;
	}
}

float UReloadComponent::GetNormalizedReloadElapsedTime() const
{
	if ( ReloadDataAsset && ReloadDataAsset->NormalReloadDuration > 0.0f )
	{
		return ReloadDataAsset->ReloadElapsedTime / ReloadDataAsset->NormalReloadDuration;
	}
	else
	{
		return 0.0f;
	}
}

void UReloadComponent::UpdateAmmoCount( int32 NewCount )
{

}

void UReloadComponent::GetAmmoData( int32& IndexMagazine, int32& MaxMagazineAmmo ) const
{

}

void UReloadComponent::GetReferences()
{

}

bool UReloadComponent::IsGunFireLocked() const
{
	return bIsReloadActive;
}

bool UReloadComponent::bIsAmmoFull() const
{
	return false;
}
