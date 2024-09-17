/* 
 * Implemented by BARRAU Benoit 
 */

#include "Gun/GunComponent.h"
#include "Gun/GunData.h"
#include "Reload/ReloadComponent.h"
#include"GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include"Kismet/KismetMathLibrary.h"

UGunComponent::UGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	GunDataAsset->CurrentAmmoInMagazine = 6;
	GunDataAsset->MaxMagazineAmmo = 6;
}

void UGunComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeAmmo();
	ACharacter* Player = UGameplayStatics::GetPlayerCharacter( this, 0 );
}

void UGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGunComponent::InitializeAmmo()
{
	GunDataAsset->CurrentAmmoInMagazine = GunDataAsset->MaxMagazineAmmo;
}

void UGunComponent::InitializeGunData( UGunData* NewGunData )
{
	if ( NewGunData )
	{
		GunDataAsset = NewGunData;
		UE_LOG( LogTemp, Warning, TEXT( "Gun Data Asset initialized !" ) );
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "Failed to initialize Gun Data Asset !" ) );
	}
}

bool UGunComponent::IsGunFull() const
{
	return GunDataAsset->CurrentAmmoInMagazine >= GunDataAsset->MaxMagazineAmmo;
}

bool UGunComponent::IsGunEmpty() const
{
	return GunDataAsset->CurrentAmmoInMagazine == 0;
}

bool UGunComponent::CanFire()
{
	// Access the reload system component
	if ( ReloadSystemComponent && ReloadSystemComponent->IsGunFireLocked() )
	{
		// DEBUG ONLY
		UKismetSystemLibrary::PrintString( this, TEXT( "Gun : Fire is locked from Reload System !" ), true, true, FLinearColor(1, 0, 0), 2.0f);
		return false;
	}

	// Check if the gun is empty
	bool bIsGunEmpty = IsGunEmpty();

	// Inverse the result of bIsGunEmpty(if the gun is not empty it can shoot)
	if ( bIsGunEmpty )
	{
		// Can shoot
		return true;
	}
	else
	{
		// Print out that the gun need to reload
		UKismetSystemLibrary::PrintString( this, TEXT( "You need to reload !" ), true, true, FLinearColor(0, 0.66f, 0), 2.0f);
		return false;
	}
	return false;
}

FVector UGunComponent::CalculateWorldlocation( bool UseMovementImprecision )
{
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();
	FVector CameraForward = PlayerCharacter->FindComponentByClass<UCameraComponent>()->GetForwardVector();

	// Multiply the forward vector of camera
	FVector ForwardVector = CameraForward * 30000;

	// Apply imprecision factor based on the movement
	FVector Imprecision = GetImprecisionAdjustedVector( UseMovementImprecision );

	// Calculate the starting position and end trace
	FVector StartLocation = PlayerLocation + Imprecision;
	FVector EndLocation = StartLocation + ForwardVector;

	// Line trace in the world
	FHitResult HitResult;
	bool bHit = UKismetSystemLibrary::LineTraceSingle( this, StartLocation, EndLocation,
	ETraceTypeQuery::TraceTypeQuery3, false, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true );

	// If trace detect an object, we return the impact point position, otherwise the end trace position
	if ( bHit )
	{
		return HitResult.ImpactNormal;
	}
	else
	{
		return EndLocation;
	}
}

// Adjust the vector with the imprecision movement
FVector UGunComponent::GetImprecisionAdjustedVector( bool UseMovementImprecision )
{
	if ( UseMovementImprecision )
	{
		// Random imprecision values
		float RandomX = FMath::RandRange( -1000, 1000 );
		float RandomY = FMath::RandRange( -1000, 1000 );
		float RandomZ = FMath::RandRange( -1000, 1000 );

		// Create Imprecision Vector
		return FVector( RandomX, RandomY, RandomZ );
	}

	return FVector::ZeroVector;
}

