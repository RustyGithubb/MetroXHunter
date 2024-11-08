/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/ZeroEnemy.h"
#include "AI/AISubstateManagerComponent.h"
#include "AI/AISubstate.h"
#include "AI/SpitProjectile.h"

#include "Light/LightManagerComponent.h"
#include "Health/HealthComponent.h"
#include "Electricity/ElectrocutableComponent.h"

#include "Library/UtilityLibrary.h"
#include "Library/GameplayLibrary.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "Perception/PawnSensingComponent.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AZeroEnemy::AZeroEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	BulbMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BulbMeshComponent" ) );
	BulbMeshComponent->SetupAttachment( RootComponent );

	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "HealthComponent" ) );

	ElectrocutableComponent = CreateDefaultSubobject<UElectrocutableComponent>( TEXT( "ElectrocutableComponent" ) );

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>( TEXT( "PawnSensingComponent" ) );
}

void AZeroEnemy::BeginPlay()
{
	DefaultMeshRelativeTransform = GetMesh()->GetRelativeTransform();
	DefaultMeshCollisions = GetMesh()->GetCollisionResponseToChannels();

	RetrieveReferences();

	UpdateWalkSpeed();

	GenerateBulb();
	CloseBulb();

	if ( bStartFakingDeath )
	{
		FakeDeath();
	}
	else
	{
		SimulateMeshBonesPhysics( true );
	}

	Super::BeginPlay();
}

void AZeroEnemy::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	switch ( State )
	{
		case EZeroEnemyState::RushAttackResolve:
		case EZeroEnemyState::Stun:
		{
			float Frequency = bUseStunAnimation
				? Data->StunAnimationFrequency
				: Data->SubstateSwitchedAnimationFrequency;
			float Angle = bUseStunAnimation
				? Data->StunAnimationAngle
				: Data->SubstateSwitchedAnimationAngle;

			double AngleOffset = FMath::Sin(
				GetGameTimeSinceCreation() * Frequency
			) * Angle;

			SetActorRotation(
				FRotator {
					0.0,
					StartStunRotation.Yaw + AngleOffset,
					0.0
				}
			);

			UE_VLOG( this, LogTemp, Verbose, TEXT( "Stun Tick" ) );
			break;
		}
		case EZeroEnemyState::RushAttack:
		{
			// Get current rush time
			FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			float CurrentRushTime = TimerManager.GetTimerElapsed( RushTimerHandle );

			// Apply new walk speed
			UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
			MovementComponent->MaxWalkSpeed = Data->RushSpeedCurve->GetFloatValue( CurrentRushTime );

			// Move forward
			AddMovementInput( GetActorForwardVector() );

			UE_VLOG( this, LogTemp, Verbose, TEXT( "Rush Tick" ) );
			break;
		}
	}
}

void AZeroEnemy::FakeDeath()
{
	SetState( EZeroEnemyState::FakingDeath );

	// Ragdoll mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( true );
	MeshComponent->SetCollisionResponseToChannels( Data->MeshRagdollCollisions );

	// Disable tick and character movement
	SetActorTickEnabled( false );
	GetCharacterMovement()->SetActive( false );

	SetCollisionsEnabled( false );
}

void AZeroEnemy::UnFakeDeath()
{
	ensureMsgf(
		State == EZeroEnemyState::FakingDeath,
		TEXT( "ZeroEnemy: %s is supposed to be in the FakingDeath state" ),
		*GetName()
	);

	SetState( EZeroEnemyState::None );

	// Un-ragdoll mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( false );
	// NOTE: We must re-attach the mesh to the root component because simulating physics
	// de-attach components from their parent.
	MeshComponent->AttachToComponent( RootComponent, FAttachmentTransformRules::KeepRelativeTransform );
	MeshComponent->SetRelativeTransform( DefaultMeshRelativeTransform );
	MeshComponent->SetCollisionResponseToChannels( DefaultMeshCollisions );

	// Enable tick and character movement
	SetActorTickEnabled( true );
	GetCharacterMovement()->SetActive( true );

	SimulateMeshBonesPhysics( true );
	SetCollisionsEnabled( true );
}

void AZeroEnemy::SimulateMeshBonesPhysics( bool bSimulate )
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	for ( const FName BoneName : SimulatedMeshBones )
	{
		MeshComponent->SetAllBodiesBelowSimulatePhysics( BoneName, bSimulate );
		MeshComponent->SetEnableGravityOnAllBodiesBelow( bSimulate, BoneName );
	}
}

void AZeroEnemy::SetCollisionsEnabled( bool bEnabled )
{
	ECollisionEnabled::Type CollisionType = bEnabled
		? ECollisionEnabled::QueryAndPhysics
		: ECollisionEnabled::NoCollision;

	GetCapsuleComponent()->SetCollisionEnabled( CollisionType );
	BulbMeshComponent->SetCollisionEnabled( CollisionType );
}

void AZeroEnemy::OpenBulb( float OpenTime )
{
	BulbMeshComponent->SetMaterial( 0, Data->OpenedBulbMaterial );

	// Enable aim assist on bulb
	BulbMeshComponent->SetCollisionObjectType( Data->AimAssistCollisionChannel );

	if ( OpenTime > 0.0f )
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimer(
			OpeningBulbTimerHandle,
			this, &AZeroEnemy::CloseBulb,
			OpenTime
		);
	}

	bIsBulbOpened = true;

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Open Bulb" ) );
}

void AZeroEnemy::CloseBulb()
{
	BulbMeshComponent->SetMaterial( 0, Data->ClosedBulbMaterial );

	// Disable aim assist on bulb
	BulbMeshComponent->SetCollisionObjectType( Data->DefaultBodyPartCollisionChannel );

	// Clear potential on-going timer
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer( OpeningBulbTimerHandle );

	bIsBulbOpened = false;

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Close Bulb" ) );
}

void AZeroEnemy::Stun( float StunTime, bool bUseDefaultAnimation )
{
	SetState( EZeroEnemyState::Stun );
	bUseStunAnimation = bUseDefaultAnimation;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(
		StunTimerHandle,
		this, &AZeroEnemy::UnStun,
		StunTime
	);

	StartStunRotation = GetActorRotation();

	OnStun.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Start Stun" ) );
}

void AZeroEnemy::UnStun()
{
	SetState( EZeroEnemyState::None );

	GetWorld()->GetTimerManager().ClearTimer( StunTimerHandle );

	OnUnStun.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Stop Stun" ) );
}

void AZeroEnemy::MakePanic()
{
	// Force un-faking death
	if ( State == EZeroEnemyState::FakingDeath )
	{
		UnFakeDeath();
	}

	OpenBulb( Data->PanicBulbOpenTime );
	Stun( Data->PanicStunTime );
}

bool AZeroEnemy::DestroyBodyPart(
	USceneComponent* BodyPart,
	const FName& BoneName,
	const FVector& KnockbackDirection
)
{
	// Force un-faking death
	if ( State == EZeroEnemyState::FakingDeath )
	{
		UnFakeDeath();
	}

	if ( auto SkeletalBodyPart = Cast<USkeletalMeshComponent>( BodyPart ) )
	{
		// Ragdoll all bones below the hit bone name
		SkeletalBodyPart->SetAllBodiesBelowSimulatePhysics( BoneName, true );
		SkeletalBodyPart->SetAllBodiesBelowLinearVelocity( BoneName, KnockbackDirection * 500.0f );

		const int32 RootIndex = SkeletalBodyPart->FindRootBodyIndex();
		const int32 BoneIndex = SkeletalBodyPart->GetBoneIndex( BoneName );
		const bool bIsBoneRoot = RootIndex == BoneIndex;
		if ( Data->bPanicOnlyIfDismembered && !bIsBoneRoot ) return false;

		if ( bIsBoneRoot )
		{
			SkeletalBodyPart->SetCollisionResponseToChannels( Data->BodyPartRagdollCollisions );
			SkeletalBodyPart->DetachFromComponent( FDetachmentTransformRules::KeepWorldTransform );
		}
	}
	else
	{
		BodyPart->DestroyComponent();
	}

	LeftBodyPartsCount -= 1;
	if ( LeftBodyPartsCount <= Data->BodyPartsLeftToKill ) return true;

	if ( HealthComponent->IsAlive() )
	{
		MakePanic();
		UpdateWalkSpeed();
	}

	return false;
}

int32 AZeroEnemy::GetStartingBodyPartsCount() const
{
	return StartBodyPartsCount;
}

void AZeroEnemy::ApplyKnockback( const FVector& Direction, float Force )
{
	if ( Force == 0.0f ) return;

	FVector Impulse = Direction.GetSafeNormal2D() * Force;
	Impulse.Z = Data->DefaultKnockbackZ;

	GetCharacterMovement()->AddImpulse( Impulse, true );
}

void AZeroEnemy::RushAttack()
{
	SetState( EZeroEnemyState::RushAttack );

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(
		RushTimerHandle,
		this, &AZeroEnemy::StopRushAttack,
		//Data->RushTime
		MaxRushTime
	);

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->RotationRate.Yaw = Data->RushYawRotationRate;
	// Force velocity to maximum walk speed to patch an issue with BTTask_MoveTo reseting velocity 
	// at the end of the task
	//CharacterMovement->Velocity = CharacterMovement->Velocity.GetSafeNormal() * Data->RushSpeedCurve->GetFloatValue( 0.0f );
	MovementComponent->Velocity = MovementComponent->Velocity.GetSafeNormal()
								* MovementComponent->MaxWalkSpeed;

	OnRush.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Start Rush Attack" ) );
}

void AZeroEnemy::StartResolveRushAttack()
{
	SetState( EZeroEnemyState::RushAttackResolve );

	GetWorld()->GetTimerManager().ClearTimer( RushTimerHandle );

	bUseStunAnimation = true;
	StartStunRotation = GetActorRotation();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Start resolving Rush Attack" ) );
}

void AZeroEnemy::StopRushAttack()
{
	SetState( EZeroEnemyState::None );

	GetWorld()->GetTimerManager().ClearTimer( RushTimerHandle );

	// Reset walk speed
	UpdateWalkSpeed();
	GetCharacterMovement()->RotationRate.Yaw = Data->YawRotationRate;

	OnUnRush.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Stop Rush Attack" ) );
}

void AZeroEnemy::SpitAttack( const FVector& TargetLocation )
{
	auto SpitProjectile = GetWorld()->SpawnActor<ASpitProjectile>(
		Data->SpitProjectileClass,
		GetSpitAttackOrigin(), GetActorRotation()
	);
	if ( !IsValid( SpitProjectile ) ) return;

	SpitProjectile->ComputeVelocityToLocation( TargetLocation, Data->SpitSpreadRange );
}

bool AZeroEnemy::IsBulbOpened() const
{
	return bIsBulbOpened;
}

bool AZeroEnemy::IsRushing() const
{
	return State == EZeroEnemyState::RushAttack
		|| State == EZeroEnemyState::RushAttackResolve;
}

void AZeroEnemy::Scream()
{
	// Play scream sound
	UGameplayStatics::PlaySoundAtLocation(
		this,
		Data->ScreamSound,
		GetActorLocation(),
		/* VolumeMultiplier */ 1.0f,
		/* PitchMultiplier */ 1.0f,
		/* StartTime */ 0.0f,
		Data->SoundAttenuation,
		Data->SoundConcurrency
	);

	// Flick lights
	AGameModeBase* Gamemode = GetWorld()->GetAuthGameMode();
	auto LightManagerComponent = Gamemode->GetComponentByClass<ULightManagerComponent>();
	if ( IsValid( LightManagerComponent ) )
	{
		LightManagerComponent->FlickeringLights(
			Data->ScreamFlickeringLightRadius,
			GetActorLocation(),
			Data->ScreamFlickeringLightCurve
		);
	}
}

void AZeroEnemy::ApplyModifiers( const FZeroEnemyModifiers& NewModifiers )
{
	Modifiers = NewModifiers;

	UpdateWalkSpeed();
}

void AZeroEnemy::ResetModifiers()
{
	ApplyModifiers( {} );
}

void AZeroEnemy::SetState( EZeroEnemyState NewState )
{
	EZeroEnemyState OldState = State;
	State = NewState;
	OnStateUpdate.Broadcast( NewState, OldState );
}

EZeroEnemyState AZeroEnemy::GetState() const
{
	return State;
}

bool AZeroEnemy::CanCallTakeDamage_Implementation( const FDamageContext& DamageContext )
{
	// NOTE: We want to allow the call no matter what, even if already dead.
	//		 Because we want to allow players to dismember dead bodies and to bring
	//		 consistency in gameplay with death faker enemies.
	return true;
}

bool AZeroEnemy::TakeDamage_Implementation( FDamageContext& DamageContext )
{
	UPrimitiveComponent* HitComponent = DamageContext.HitResult.GetComponent();

	const FVector KnockbackDirection = UKismetMathLibrary::GetDirectionUnitVector(
		DamageContext.HitResult.TraceStart,
		DamageContext.HitResult.TraceEnd
	);

	// Check if damaged the bulb
	if ( HitComponent == BulbMeshComponent )
	{
		if ( IsBulbOpened() )
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				this,
				Data->BulbHitNiagara,
				BulbMeshComponent->GetComponentLocation() - KnockbackDirection * Data->BulbHitNiagaraDistance,
				BulbMeshComponent->GetComponentRotation()
			);
			return true;
		}

		return false;
	}

	// Check if damaged one of its body part
	if ( IsValid( HitComponent ) && HitComponent->ComponentHasTag( Data->BodyPartTag ) )
	{
		bool bIsDead = DestroyBodyPart(
			HitComponent,
			DamageContext.HitResult.BoneName,
			KnockbackDirection
		);
		if ( bIsDead )
		{
			// Force kill this actor
			DamageContext.DamageAmount = HealthComponent->CurrentHealth;
			return true;
		}

		ApplyKnockback( KnockbackDirection, Data->BodyPartHitKnockbackForce );
		return false;
	}

	// Knockback for all damage except electricity
	if ( DamageContext.DamageType != EDamageType::Shock )
	{
		ApplyKnockback( KnockbackDirection, Data->WholeBodyHitKnockbackForce );
	}

	return false;
}

#if ENABLE_VISUAL_LOG
void AZeroEnemy::GrabDebugSnapshot( FVisualLogEntry* Snapshot ) const
{
	FVisualLogStatusCategory Category( TEXT( "ZeroEnemy" ) );
	Category.Add(
		TEXT( "State" ),
		UEnum::GetValueAsString( GetState() )
	);
	Category.Add(
		TEXT( "LeftBodyPartsCount" ),
		FString::FromInt( LeftBodyPartsCount )
	);
	Category.Add(
		TEXT( "StartBodyPartsCount" ),
		FString::FromInt( StartBodyPartsCount )
	);

	Snapshot->Status.Add( Category );
}
#endif

void AZeroEnemy::MeleeAttack_Implementation()
{
	SetState( EZeroEnemyState::MeleeAttack );
	OnMeleeAttack.Broadcast( true );

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Start Melee Attack" ) );
}

void AZeroEnemy::StopMeleeAttack_Implementation()
{
	SetState( EZeroEnemyState::None );
	OnMeleeAttack.Broadcast( false );

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Stop Melee Attack" ) );
}

void AZeroEnemy::GenerateBulb()
{
	auto BulbSpots = UUtilityLibrary::GetComponentsOfActorByTag<UArrowComponent>(
		this,
		Data->BulbSpotTag
	);

	auto PickedBulbSpot = UUtilityLibrary::PickRandomElement( BulbSpots );
	BulbMeshComponent->AttachToComponent(
		PickedBulbSpot,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);
}

void AZeroEnemy::RetrieveReferences()
{
	verifyf( IsValid( Data ), TEXT( "%s doesn't reference a DataAsset" ), *GetName() );
	verifyf(
		IsValid( Data->RushSpeedCurve ),
		TEXT( "The DataAsset must set a value for 'RushSpeedCurve'" )
	);

	// Retrieve maximum rush time from the curve
	[[maybe_unused]] float Temp = 0.0f;
	Data->RushSpeedCurve->GetTimeRange( Temp, MaxRushTime );

	auto BodyParts = UUtilityLibrary::GetComponentsOfActorByTag<UMeshComponent>(
		this,
		Data->BodyPartTag
	);
	StartBodyPartsCount = BodyParts.Num();
	LeftBodyPartsCount = StartBodyPartsCount;
	checkf(
		StartBodyPartsCount > 0,
		TEXT( "ZeroEnemy %s doesn't have any body parts count!" ),
		*GetName()
	);

	// Enable aim assist for all body parts
	for ( auto BodyPart : BodyParts )
	{
		if ( Data->bBodyPartHasAimAssist )
		{
			BodyPart->SetCollisionObjectType( Data->AimAssistCollisionChannel );
		}

		BodyPart->SetCollisionResponseToChannels( Data->BodyPartDefaultCollisions );
	}

	// Set max health
	HealthComponent->MaxHealth = Data->MaxHealth;
	
	HealthComponent->OnDeath.AddDynamic( this, &AZeroEnemy::OnDeath );
	ElectrocutableComponent->OnElectricStart.AddDynamic( this, &AZeroEnemy::OnElectricStart );
}

void AZeroEnemy::UpdateWalkSpeed()
{
	float WalkSpeed = Data->WalkSpeed * Modifiers.WalkSpeedMultiplier;

	// Substract speed for each body parts lost
	WalkSpeed -= Data->WalkSpeedLossPerBodyPartLost * ( StartBodyPartsCount - LeftBodyPartsCount );

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AZeroEnemy::OnElectricStart( float Duration )
{
	if ( State == EZeroEnemyState::FakingDeath )
	{
		UnFakeDeath();
	}
}

void AZeroEnemy::OnDeath( const FDamageContext& DamageContext )
{
	CloseBulb();

	if ( IsValid( Controller ) )
	{
		Controller->Destroy();
	}

	// Disable tick and character movement
	SetActorTickEnabled( false );
	GetCharacterMovement()->SetActive( false );

	// Simulate physics and setup collisions on body mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( true );
	MeshComponent->SetCollisionResponseToChannels( Data->MeshRagdollCollisions );
	MeshComponent->SetReceivesDecals( false );
	MeshComponent->OnComponentHit.AddDynamic( this, &AZeroEnemy::OnRagdollMeshHit );

	BulbMeshComponent->SetReceivesDecals( false );

	// Apply knockback to mesh
	const FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(
		DamageContext.HitResult.TraceStart,
		DamageContext.HitResult.TraceEnd
	);
	if ( !Direction.IsNearlyZero() )
	{
		const FVector Knockback = Direction * Data->DeathKnockbackForce;
		//MeshComponent->AddImpulseAtLocation( Knockback, DamageContext.HitResult.ImpactPoint );
		MeshComponent->SetAllPhysicsLinearVelocity( Knockback, true );
	}

	SetCollisionsEnabled( false );
}

void AZeroEnemy::OnRagdollMeshHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	// NOTE: It is assumed that this function is triggered only after death
	//		 when the mesh hit something.

	const float VelocityLength = HitComponent->GetComponentVelocity().Length();
	if ( VelocityLength > Data->BloodPuddleSpawnMaxVelocity ) return;

	SpawnBloodPuddle();

	GetMesh()->OnComponentHit.RemoveDynamic( this, &AZeroEnemy::OnRagdollMeshHit );
}

void AZeroEnemy::SpawnBloodPuddle()
{
	UGameplayLibrary::SpawnBloodPuddleAtBone(
		this,
		Data->BloodPuddleClass,
		GetMesh(),
		Data->BloodPuddleSpawnBoneName,
		Data->BloodPuddleScale
	);
}
