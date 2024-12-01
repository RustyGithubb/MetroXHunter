/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Vent/Vent.h"
#include "Vent/VentData.h"

#include "AI/Parasite.h"

#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

#include "Library/UtilityLibrary.h"

AVent::AVent()
{
	PrimaryActorTick.bCanEverTick = false;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>( TEXT( "RootComponent" ) );
	RootComponent = DefaultSceneRoot;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "MeshComponent" ) );
	MeshComponent->SetupAttachment( RootComponent );
}

void AVent::BeginPlay()
{
	Super::BeginPlay();

	verifyf( IsValid( DataAsset ), TEXT( "Vent %s doesn't have a valid data asset!" ), *GetName() );
}

void AVent::EnterVent( AActor* Actor )
{
	verify( IsValid( Actor ) );

	Actor->Destroy();

	verify( IsValid( VentManager ) );

	VentManager->AddEnemyInZone( Zone, 1 );
	VentManager->ResetProximitySpawnTimer();
}

void AVent::TrySpawnEnemies()
{
	const int32 EnemyCounter = VentManager->GetEnemiesCounterInZone( Zone );
	if ( EnemyCounter == 0 ) return;

	const int32 ToSpawnAmount = FMath::RandRange(
		1, FMath::Min( DataAsset->MaxEnemiesToSpawn, EnemyCounter )
	);

	float SpawnTime = 0.0f;
	for ( int Index = 0; Index < ToSpawnAmount; Index++ )
	{
		// Accumulate delay to spawn each enemy one after another
		SpawnTime += UUtilityLibrary::RandomInRange( DataAsset->SpawnTimeRange );

		FTimerHandle TimerHandle {};
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			this, &AVent::SpawnEnemy,
			SpawnTime
		);
	}

	VentManager->RemoveEnemyInZone( Zone, ToSpawnAmount );
	VentManager->ResetProximitySpawnTimer();
}

void AVent::PlayEnterSound()
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		DataAsset->EnterSound,
		GetEntranceComponent()->GetComponentLocation(),
		/* VolumeMultiplier */ 0.5f,
		/* PitchMultiplier */ 0.3f,
		/* StartTime */ 0.0f,
		DataAsset->SoundAttenuation,
		DataAsset->SoundConcurrency
	);
}

void AVent::PlayIdleSound()
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		UUtilityLibrary::PickRandomElement( DataAsset->IdleSounds ),
		GetSpawnPointComponent()->GetComponentLocation(),
		DataAsset->IdleSoundVolumeMultiplier,
		/* PitchMultiplier */ 0.2f,
		/* StartTime */ 0.0f,
		DataAsset->SoundAttenuation,
		DataAsset->SoundConcurrency
	);
}

void AVent::PlayScreamSound()
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		UUtilityLibrary::PickRandomElement( DataAsset->IdleSounds ),
		GetSpawnPointComponent()->GetComponentLocation(),
		DataAsset->IdleSoundVolumeMultiplier,
		/* PitchMultiplier */ 0.2f,
		/* StartTime */ 0.0f,
		DataAsset->SoundAttenuation,
		DataAsset->SoundConcurrency
	);
}

void AVent::SpawnEnemy()
{
	FActorSpawnParameters SpawnParams {};
	//	NOTE: I set the owner so the ParasiteAIController can use GetOwner to set a cooldown
	//		  for the next vent use on its own.
	SpawnParams.Owner = this;
	//	NOTE: Since we spawn the enemy in a wall, the collision handler would always fail to spawn
	//		  the actor so we force it to spawn no matter what.
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform SpawnTransform(
		GetActorRotation(),
		GetSpawnPointComponent()->GetComponentLocation(),
		FVector { 1.0f, 1.0f, 1.0f }
	);

	//	Spawn enemy actor
	AParasite* Enemy = GetWorld()->SpawnActor<AParasite>(
		DataAsset->EnemyClass,
		SpawnTransform,
		SpawnParams
	);
	verify( IsValid( Enemy ) );

	//	Start enemy's animation to exit the vent
	Enemy->StartExitingVent( this );
}
