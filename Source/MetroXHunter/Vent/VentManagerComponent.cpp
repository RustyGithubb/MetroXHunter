/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Vent/VentManagerComponent.h"
#include "Vent/Vent.h"
#include "Vent/VentData.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include "Library/UtilityLibrary.h"

UVentManagerComponent::UVentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UVentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	verifyf(
		IsValid( DataAsset ),
		TEXT( "VentManager %s doesn't have a valid data asset!" ),
		*GetName()
	);

	CreateNextIdleSoundTimer();

	// Auto-start cyclic spawn with pre-filled zones enemies amount
	if ( ShouldCyclicSpawnTimerRunning() )
	{
		StartCyclicSpawnTimer();
	}
}

void UVentManagerComponent::TickDebug_Implementation( float DeltaTime, FString& OutDebugText )
{
	// Constructs counter string 
	FString EnemiesCounter = "";
	for ( int Index = 0; Index < (int32)EVentZone::MAX; Index++ )
	{
		EnemiesCounter += "- " + UEnum::GetValueAsString( (EVentZone)Index ) + ": "
						+ FString::FromInt( ZoneEnemies[Index] ) + "\n";
	}

	const FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	float ProximitySpawnTime = TimerManager.GetTimerRemaining( ProximitySpawnTimerHandle );
	float CyclicSpawnTime = TimerManager.GetTimerRemaining( CyclicSpawnTimerHandle );
	float IdleSoundTime = TimerManager.GetTimerRemaining( IdleSoundTimerHandle );

	// Constructs formating arguments
	FStringFormatNamedArguments Args {};
	Args.Add( "VentsCount", Vents.Num() );
	Args.Add( "ProximitySpawnTime", ProximitySpawnTime );
	Args.Add( "CyclicSpawnTime", CyclicSpawnTime );
	Args.Add( "IsCyclicSpawnTimerRunning", TimerManager.IsTimerActive( CyclicSpawnTimerHandle ) );
	Args.Add( "ShouldCyclicSpawnTimerRunning", ShouldCyclicSpawnTimerRunning() );
	Args.Add( "IdleSoundTime", IdleSoundTime );
	Args.Add( "EnemiesCounter", EnemiesCounter );

	// Formats debug string
	constexpr auto Format = TEXT(
		"[VentManagerComponent]\n"
		"VentsCount: {VentsCount}\n"
		"ProximitySpawnTime: {ProximitySpawnTime}\n"
		"CyclicSpawnTime: {CyclicSpawnTime}s\n"
		"IsCyclicSpawnTimerRunning: {IsCyclicSpawnTimerRunning}\n"
		"ShouldCyclicSpawnTimerRunning: {ShouldCyclicSpawnTimerRunning}\n"
		"IdleSoundTime: {IdleSoundTime}s\n"
		"EnemiesCounter:\n"
		"{EnemiesCounter}\n"
	);
	OutDebugText = FString::Format( Format, Args );
}

int32 UVentManagerComponent::GetEnemiesCounterInZone( EVentZone Zone ) const
{
	const int32 Index = (int32)Zone;
	verify( Index >= 0 && Index < (int32)EVentZone::MAX );

	return ZoneEnemies[Index];
}

void UVentManagerComponent::StartCyclicSpawnTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		CyclicSpawnTimerHandle,
		this, &UVentManagerComponent::OnDepleteCyclicSpawnTimer,
		UUtilityLibrary::RandomInRange( DataAsset->CyclicSpawnTimeRange )
	);
}

void UVentManagerComponent::StopCyclicSpawnTimer()
{
	GetWorld()->GetTimerManager().ClearTimer( CyclicSpawnTimerHandle );
}

bool UVentManagerComponent::IsZoneHasEnoughEnemiesForCyclicSpawn( EVentZone Zone ) const
{
	return GetEnemiesCounterInZone( Zone ) >= DataAsset->MinEnemiesInZoneToStartCyclicSpawn;
}

bool UVentManagerComponent::ShouldCyclicSpawnTimerRunning() const
{
	for ( int Index = 0; Index < (int)EVentZone::MAX; Index++ )
	{
		if ( IsZoneHasEnoughEnemiesForCyclicSpawn( (EVentZone)Index ) )
		{
			return true;
		}
	}

	return false;
}

void UVentManagerComponent::ResetProximitySpawnTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		ProximitySpawnTimerHandle,
		UUtilityLibrary::RandomInRange( DataAsset->ProximitySpawnTimeRange ),
		false
	);
}

bool UVentManagerComponent::IsProximitySpawnTimerDepleted() const
{
	return GetWorld()->GetTimerManager().GetTimerRemaining( ProximitySpawnTimerHandle ) <= 0.0f;
}

void UVentManagerComponent::AddEnemyInZone( EVentZone Zone, int Amount )
{
	const int32 Index = (int32)Zone;
	verify( Index >= 0 && Index < (int32)EVentZone::MAX );

	ZoneEnemies[Index] += Amount;

	UUtilityLibrary::LogMessage(
		TEXT( "%s: Add %d enemy in zone %s" ),
		*GetName(),
		Amount,
		*UEnum::GetValueAsString( Zone )
	);

	// NOTE: Not calling ShouldCyclicSpawnTimerRunning here to avoid checking all zones when
	//		 only this zone has changed from last time.
	if ( !GetWorld()->GetTimerManager().IsTimerActive( CyclicSpawnTimerHandle )
	  && IsZoneHasEnoughEnemiesForCyclicSpawn( Zone ) )
	{
		StartCyclicSpawnTimer();
	}
}

void UVentManagerComponent::RemoveEnemyInZone( EVentZone Zone, int Amount )
{
	const int32 Index = (int32)Zone;
	verify( Index >= 0 && Index < (int32)EVentZone::MAX );

	ZoneEnemies[Index] = FMath::Max( 0, ZoneEnemies[Index] - Amount );

	UUtilityLibrary::LogMessage(
		TEXT( "%s: Remove %d enemy in zone %s" ),
		*GetName(),
		Amount,
		*UEnum::GetValueAsString( Zone )
	);

	if ( GetWorld()->GetTimerManager().IsTimerActive( CyclicSpawnTimerHandle )
		&& !ShouldCyclicSpawnTimerRunning() )
	{
		StopCyclicSpawnTimer();
	}
}

void UVentManagerComponent::RetrieveVents()
{
	Vents.Empty();

	for ( TActorIterator<AVent> It( GetWorld(), AVent::StaticClass() ); It; ++It )
	{
		AVent* Vent = *It;
		RegisterVent( Vent );
	}
}

void UVentManagerComponent::RegisterVent( AVent* Vent )
{
	verify( IsValid( Vent ) );

	Vent->VentManager = this;
	Vent->OnEndPlay.AddUniqueDynamic( this, &UVentManagerComponent::OnVentEndPlay );
	Vents.AddUnique( Vent );

	UUtilityLibrary::LogMessage(
		TEXT( "%s: Registered %s" ),
		*GetName(), *Vent->GetName()
	);

	UE_VLOG_LOCATION( 
		this,
		LogTemp, Log,
		Vent->GetActorLocation(), 32.0f,
		FColor::Emerald,
		TEXT( "%s: Registered %s" ),
		*GetName(), *Vent->GetName()
	);
}

void UVentManagerComponent::CreateNextIdleSoundTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		IdleSoundTimerHandle,
		this,
		&UVentManagerComponent::PlayIdleSound,
		UUtilityLibrary::RandomInRange( DataAsset->IdleSoundTimeRange )
	);
}

void UVentManagerComponent::PlayIdleSound()
{
	const AActor* OriginActor = GetPlayer();

	float NearestDistance = TNumericLimits<float>::Max();
	AVent* NearestVent = nullptr;
	
	// Finds nearest vent from the origin actor
	for ( const auto Vent : Vents )
	{
		const float Distance = Vent->GetDistanceTo( OriginActor );
		
		// Filter vents that are further than the current one
		if ( Distance > NearestDistance ) continue;

		// Filter vents that are too far or too close from the player
		if ( !DataAsset->IdleSoundDistanceRange.Contains( Distance ) ) continue;

		NearestVent = Vent;
		NearestDistance = Distance;
	}

	// Plays sound on this vent
	if ( IsValid( NearestVent ) )
	{
		NearestVent->PlayIdleSound();
	}

	CreateNextIdleSoundTimer();
}

AVent* UVentManagerComponent::FindMostLookedVent( const TArray<AVent*>& VentsToSearch )
{
	const AActor* OriginActor = GetPlayer();
	const float MinDotProduct = FMath::Cos( DataAsset->CyclicSpawnMostLookedVentFOV );

	float LookedDotProduct = -1.0f;
	AVent* LookedVent = nullptr;
	for ( const auto Vent : VentsToSearch )
	{
		const FVector ToVentDirection =
			( Vent->GetActorLocation() - OriginActor->GetActorLocation() ).GetSafeNormal();
		const FVector AimDirection = OriginActor->GetActorForwardVector();

		float DotProduct = FVector::DotProduct( ToVentDirection, AimDirection );
		if ( DotProduct <= LookedDotProduct ) continue;
		if ( DotProduct < MinDotProduct ) continue;

		LookedVent = Vent;
		LookedDotProduct = DotProduct;
	}

	return LookedVent;
}

AVent* UVentManagerComponent::TryChangeChoosenVent( TArray<AVent*> VentsInRange, AVent* ChoosenVent )
{
	// No need to run the algorithm if there is only one possibility
	if ( VentsInRange.Num() == 1 ) return ChoosenVent;

	const AVent* LookedVent = FindMostLookedVent( VentsInRange );

	#if ENABLE_VISUAL_LOG
	if ( LookedVent != nullptr )
	{
		UE_VLOG_LOCATION(
			this,
			LogTemp, Verbose,
			LookedVent->GetActorLocation() + FVector { 0.0f, 0.0f, 48.0f }, 32.0f,
			FColor::Cyan,
			TEXT( "VentManager: CyclicSpawn: %s is the most-looked vent" ),
			*LookedVent->GetName()
		);
	}
	#endif

	if ( ChoosenVent == LookedVent )
	{
		// Tries to randomize vent to a less looked vent
		const int32 Chance = FMath::RandRange( 1, 100 );
		if ( Chance <= DataAsset->CyclicSpawnRerollMostLookedVentChance )
		{
			#if ENABLE_VISUAL_LOG
			UE_VLOG_LOCATION(
				this,
				LogTemp, Verbose,
				ChoosenVent->GetActorLocation() + FVector { 0.0f, 0.0f, 96.0f }, 32.0f,
				FColor::Cyan,
				TEXT( "VentManager: CyclicSpawn: %s was first choosen" ),
				*ChoosenVent->GetName()
			);
			#endif

			VentsInRange.Remove( ChoosenVent );
			ChoosenVent = UUtilityLibrary::PickRandomElement( VentsInRange );

			#if ENABLE_VISUAL_LOG
			UE_VLOG_LOCATION(
				this,
				LogTemp, Verbose,
				ChoosenVent->GetActorLocation() + FVector { 0.0f, 0.0f, 96.0f }, 32.0f,
				FColor::Orange,
				TEXT(
					"VentManager: CyclicSpawn: Rolled to switch from %s to %s "
					"because it is the most-looked vent"
				),
				*LookedVent->GetName(),
				*ChoosenVent->GetName()
			);
			#endif
		}
		#if ENABLE_VISUAL_LOG
		else
		{
			UE_VLOG_LOCATION(
				this,
				LogTemp, Verbose,
				ChoosenVent->GetActorLocation() + FVector { 0.0f, 0.0f, 96.0f }, 32.0f,
				FColor::Orange,
				TEXT(
					"VentManager: CyclicSpawn: Rolled to keep %s "
					"even though it is the most-looked vent"
				),
				*ChoosenVent->GetName()
			);
		}
		#endif
	}
	#if ENABLE_VISUAL_LOG
	else
	{
		UE_VLOG_LOCATION(
			this,
			LogTemp, Verbose,
			ChoosenVent->GetActorLocation() + FVector { 0.0f, 0.0f, 96.0f }, 32.0f,
			FColor::Orange,
			TEXT(
				"VentManager: CyclicSpawn: %s has been choosen with no modifications"
			),
			*ChoosenVent->GetName()
		);
	}
	#endif

	return ChoosenVent;
}

void UVentManagerComponent::OnDepleteCyclicSpawnTimer()
{
	const AActor* OriginActor = GetPlayer();

	// Finds all vents in range from the origin actor
	TArray<AVent*> VentsInRange {};
	for ( const auto Vent : Vents )
	{
		const float Distance = Vent->GetDistanceTo( OriginActor );

		// Filters vents that are too far or too close from the player
		if ( !DataAsset->CyclicSpawnDistanceRange.Contains( Distance ) ) continue;

		VentsInRange.Add( Vent );

		#if ENABLE_VISUAL_LOG
		UE_VLOG_LOCATION(
			this,
			LogTemp, Verbose,
			Vent->GetActorLocation(), 32.0f,
			FColor::Cyan,
			TEXT( "VentManager: CyclicSpawn: %s in range" ),
			*Vent->GetName()
		);
		#endif
	}

	if ( !VentsInRange.IsEmpty() )
	{
		AVent* ChoosenVent = UUtilityLibrary::PickRandomElement( VentsInRange );
		ChoosenVent = TryChangeChoosenVent( VentsInRange, ChoosenVent );
		ChoosenVent->TrySpawnEnemies();
	}

	// Schedules next cyclic spawn
	if ( ShouldCyclicSpawnTimerRunning() )
	{
		StartCyclicSpawnTimer();
	}
}

void UVentManagerComponent::OnVentEndPlay( AActor* Actor, EEndPlayReason::Type Reason )
{
	AVent* Vent = Cast<AVent>( Actor );
	if ( Vent == nullptr ) return;

	Vents.Remove( Vent );

	UUtilityLibrary::LogMessage(
		TEXT( "%s: Un-registered vent %s" ),
		*GetName(),
		*Vent->GetName()
	);
}

APawn* UVentManagerComponent::GetPlayer()
{
	if ( !IsValid( PlayerPawn ) )
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn( this, 0 );
	}

	return PlayerPawn;
}
