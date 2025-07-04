/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/SpitProjectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Health/HealthComponent.h"
#include "Components/SphereComponent.h"

#include "Library/ConvarLibrary.h"

#include "Kismet/GameplayStatics.h"

#define VLOG_COLOR_COMPUTE		FColor::FromHex( "008DFFFF" )
#define VLOG_COLOR_EXPLOSION	FColor::FromHex( "FFAA00FF" )
#define VLOG_CATEGORY			LogTemp
#define VLOG_VERBOSITY			Verbose

ASpitProjectile::ASpitProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>( TEXT( "SphereComponent" ) );
	RootComponent = SphereComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>( 
		TEXT( "ProjectileMovementComponent" ) 
	);
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "HealthComponent" ) );
}

void ASpitProjectile::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovementComponent->OnProjectileStop.AddUniqueDynamic(
		this, &ASpitProjectile::OnProjectileStop
	);
	HealthComponent->OnDeath.AddUniqueDynamic(
		this, &ASpitProjectile::OnDeath
	);
}

void ASpitProjectile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void ASpitProjectile::ComputeVelocityToLocation( const FVector& TargetLocation, float SpreadRange )
{
	FVector& OutVelocity = ProjectileMovementComponent->Velocity;

	const FVector StartLocation = GetActorLocation();
	FVector EndLocation = TargetLocation;

	// Compute spread in a spherical shape
	if ( SpreadRange > 0.0f )
	{
		float Spread = FMath::RandRange( 0.0f, SpreadRange );
		float Angle = FMath::RandRange( -PI, PI );
		float Cos = 0.0f, Sin = 0.0f;
		FMath::SinCos( &Sin, &Cos, Angle );

		EndLocation += FVector {
			Cos * Spread,
			Sin * Spread,
			0.0f,
		};

#if ENABLE_VISUAL_LOG
		UE_VLOG_WIRESPHERE(
			this,
			VLOG_CATEGORY, VLOG_VERBOSITY,
			TargetLocation, SpreadRange,
			FColor::FromHex( "0066BCFF" ),
			TEXT( "" )
			//TEXT( "SpitProjectile: Spread Range (=%.2f)" ),
			//SpreadRange
		);
		UE_VLOG_WIRESPHERE(
			this,
			VLOG_CATEGORY, VLOG_VERBOSITY,
			TargetLocation, Spread,
			VLOG_COLOR_COMPUTE,
			TEXT( "" )
			//TEXT( "SpitProjectile: Spread (=%.2f)" ),
			//Spread
		);
		UE_VLOG(
			this,
			VLOG_CATEGORY, VLOG_VERBOSITY,
			TEXT( "SpitProjectile: Random Spread %.1f/%.1f" ),
			Spread, SpreadRange
		);
#endif
	}

	// Setup projectile parameters
	UGameplayStatics::FSuggestProjectileVelocityParameters ProjectileParams(
		this,
		StartLocation, EndLocation,
		LaunchSpeed
	);
	ProjectileParams.bFavorHighArc = false;
	ProjectileParams.CollisionRadius = 0.0f;
	ProjectileParams.OverrideGravityZ = ProjectileMovementComponent->GetGravityZ();
	ProjectileParams.TraceOption = ESuggestProjVelocityTraceOption::DoNotTrace;
	ProjectileParams.ResponseParam = FCollisionResponseParams::DefaultResponseParam;
	ProjectileParams.ActorsToIgnore = {};
	ProjectileParams.bDrawDebug = UConvarLibrary::IsAIDebugConvarEnabled();
	ProjectileParams.bAcceptClosestOnNoSolutions = true;

	// Compute velocity
	bool bHasSolution = UGameplayStatics::SuggestProjectileVelocity( 
		ProjectileParams,
		OutVelocity
	);

#if ENABLE_VISUAL_LOG
	UE_VLOG_ARROW( 
		this,
		VLOG_CATEGORY, VLOG_VERBOSITY,
		StartLocation, StartLocation + OutVelocity,
		VLOG_COLOR_COMPUTE,
		TEXT( "SpitProjectile: Velocity" )
	);
	UE_VLOG_LOCATION(
		this,
		VLOG_CATEGORY, VLOG_VERBOSITY,
		StartLocation, 16.0f,
		VLOG_COLOR_COMPUTE,
		TEXT( "SpitProjectile: Start Location" )
	);

	if ( SpreadRange > 0.0f )
	{
		UE_VLOG_LOCATION(
			this,
			VLOG_CATEGORY, VLOG_VERBOSITY,
			TargetLocation, 16.0f,
			VLOG_COLOR_COMPUTE,
			TEXT( "SpitProjectile: Target Location" )
		);
	}

	UE_VLOG_LOCATION(
		this,
		VLOG_CATEGORY, VLOG_VERBOSITY,
		EndLocation, 16.0f,
		VLOG_COLOR_COMPUTE,
		TEXT( "SpitProjectile: End Location" )
	);

	UE_VLOG(
		this,
		VLOG_CATEGORY, VLOG_VERBOSITY,
		TEXT( "SpitProjectile: Shoot at %s with velocity of %s (Length=%f; bHasSolution=%d)" ),
		*EndLocation.ToString(),
		*OutVelocity.ToString(),
		OutVelocity.Length(),
		bHasSolution
	);
#endif
}

void ASpitProjectile::Explode( const FHitResult& ExplosionHitResult )
{
	const FVector ExplosionOrigin = GetActorLocation();

	// Get exploded actors
	TArray<AActor*> HitActors {};
	UKismetSystemLibrary::SphereOverlapActors(
		this,
		ExplosionOrigin, ExplosionRadius,
		ExplosionObjectTypes,
		nullptr,
		/* ActorsToIgnore */ {},
		HitActors
	);

#if ENABLE_VISUAL_LOG
	UE_VLOG_WIRESPHERE(
		this,
		VLOG_CATEGORY, VLOG_VERBOSITY,
		ExplosionOrigin, ExplosionRadius,
		VLOG_COLOR_EXPLOSION,
		TEXT( "SpitProjectile: Explosion" )
	);
#endif

	// Deal damage to hit actors having a HealthComponent
	for ( AActor* HitActor : HitActors )
	{
		auto HitHealthComponent = HitActor->GetComponentByClass<UHealthComponent>();
		if ( !IsValid( HitHealthComponent ) ) continue;

		// Get actor's eyes origin for a line trace
		FVector ActorEyesOrigin {};
		FRotator ActorEyesRotation {};
		HitActor->GetActorEyesViewPoint( ActorEyesOrigin, ActorEyesRotation );

		// Line trace to actor to prevent getting damaged while hiding behind a wall or a pillar
		FHitResult BlockingHitResult {};
		UKismetSystemLibrary::LineTraceSingleForObjects(
			this,
			ExplosionOrigin, ActorEyesOrigin,
			ExplosionObjectTypes,
			/* bTraceComplex */ false,
			TArray<AActor*> {},
			EDrawDebugTrace::None,
			BlockingHitResult,
			/* bIgnoreSelf */ true
		);
		if ( !BlockingHitResult.bBlockingHit || BlockingHitResult.GetActor() != HitActor ) continue;


		// Deal damage to actor
		FDamageContext DamageContext {};
		DamageContext.DamageAmount = ExplosionDamage;
		DamageContext.DamageType = EDamageType::Spit;
		DamageContext.AttackerActor = GetOwner();
		DamageContext.InflictorActor = this;
		DamageContext.HitResult = {};
		bool bTookDamage = HitHealthComponent->TakeDamage( DamageContext );

#if ENABLE_VISUAL_LOG
		const FVector HitActorLocation = HitActor->GetActorLocation();
		UE_VLOG_LOCATION(
			this,
			VLOG_CATEGORY, VLOG_VERBOSITY,
			HitActorLocation,
			32.0f,
			VLOG_COLOR_EXPLOSION,
			TEXT( "SpitProjectile: Exploded %s with %d damage (bTookDamage=%d)" ),
			*GetNameSafe( HitActor ),
			DamageContext.DamageAmount,
			bTookDamage
		);
#endif
	}

	OnExploded( ExplosionHitResult );
}

void ASpitProjectile::OnExploded_Implementation( const FHitResult& ImpactResult )
{
	Destroy();
}

void ASpitProjectile::OnProjectileStop( const FHitResult& ImpactResult )
{
	Explode( ImpactResult );
}

void ASpitProjectile::OnDeath( const FDamageContext& DamageContext )
{
	Explode( DamageContext.HitResult );
}

