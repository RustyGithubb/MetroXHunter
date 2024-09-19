/* 
 * Implemented by BARRAU Benoit 
 */

#include "Gun/GunComponent.h"
#include "Gun/GunData.h"
#include "Gun/GunCommunication.h"
#include "Reload/ReloadComponent.h"
#include"GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include"Kismet/KismetMathLibrary.h"

UGunComponent::UGunComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGunComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeAmmo();


	ACharacter* LocalPlayerCharacter = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if ( PlayerCharacter )
	{
		this->PlayerCharacter = LocalPlayerCharacter;

		ReloadSystemComponent = Cast<UReloadComponent>( PlayerCharacter->GetComponentByClass( UReloadComponent::StaticClass() ));

		if ( ReloadSystemComponent )
		{
			UE_LOG( LogTemp, Warning, TEXT( "Reload System initialized !" ) );
		}
		else
		{
			UE_LOG( LogTemp, Error, TEXT( "Reload System not initialized !" ) );
		}
	}
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
	if ( ReloadSystemComponent && ReloadSystemComponent->IsGunFireLocked() )
	{
		UKismetSystemLibrary::PrintString( this, TEXT( "Gun : Fire is locked from Reload System!" ), true, true, FLinearColor( 1, 0, 0 ), 2.0f );
		return false;
	}

	bool bIsGunEmpty = IsGunEmpty();
	if ( bIsGunEmpty )
	{
		UKismetSystemLibrary::PrintString( this, TEXT( "You need to reload!" ), true, true, FLinearColor( 0, 0.66f, 0 ), 2.0f );
		return false;
	}
	return true;
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

void UGunComponent::SetNewAmmoCount( int NewCount )
{
	// Update CurrentAmmoInMagazine with the new value
	GunDataAsset->CurrentAmmoInMagazine = NewCount;

	UE_LOG( LogTemp, Warning, TEXT( "Ammo Count updated" ), NewCount);
}

void UGunComponent::CallOnFire()
{
	if ( CanFire() )
	{
		// Do the event OnFire
		OnFire.Broadcast();

		UE_LOG( LogTemp, Warning, TEXT( "Gun fired !" ));
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( " Cannot fire !" ) );
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

