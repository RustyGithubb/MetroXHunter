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
#include "Library/ConvarLibrary.h"

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

	SaveComponent = CreateDefaultSubobject<USaveLoadComponent>( TEXT( "SaveComponent" ) );
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

			// Fixing the enemy not looking the player during the QTE
			if ( auto AIController = GetController<AAIController>() )
			{
				StartStunRotation.Yaw = AIController->GetControlRotation().Yaw;
			}

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

	Ragdoll();
}

void AZeroEnemy::UnFakeDeath()
{
	ensureMsgf(
		State == EZeroEnemyState::FakingDeath,
		TEXT( "ZeroEnemy: %s is supposed to be in the FakingDeath state" ),
		*GetName()
	);

	SetState( EZeroEnemyState::None );

	UnRagdoll();
}

void AZeroEnemy::KnockOut()
{
	if ( State != EZeroEnemyState::KnockOut )
	{
		SetState( EZeroEnemyState::KnockOut );
		Ragdoll();
	}

	GetWorldTimerManager().SetTimer( KnockOutTimerHandle, this, &AZeroEnemy::UnKnockOut, 5.0f );
}

void AZeroEnemy::UnKnockOut()
{
	SetState( EZeroEnemyState::None );
	UnRagdoll();

	GetWorldTimerManager().ClearTimer( KnockOutTimerHandle );
}

void AZeroEnemy::Ragdoll()
{
	// Ragdoll mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( true );
	MeshComponent->SetCollisionResponseToChannels( Data->MeshRagdollCollisions );

	// Disable tick and character movement
	SetActorTickEnabled( false );
	GetCharacterMovement()->SetActive( false );

	SetCollisionsEnabled( false );
}

void AZeroEnemy::UnRagdoll()
{
	// Un-ragdoll mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( false );
	// NOTE: We must re-attach the mesh to the root component because simulating physics
	// de-attach components from their parent.
	MeshComponent->AttachToComponent( RootComponent, FAttachmentTransformRules::KeepRelativeTransform );
	MeshComponent->SetRelativeTransform( DefaultMeshRelativeTransform );
	MeshComponent->SetCollisionResponseToChannels( DefaultMeshCollisions );

	// Enable tick and character movement
	GetCharacterMovement()->SetActive( true );

	SimulateMeshBonesPhysics( true );
	SetCollisionsEnabled( true );
	SetActorTickEnabled( true );
}

void AZeroEnemy::SimulateMeshBonesPhysics( bool bSimulate )
{
	USkeletalMeshComponent* MeshComponent = GetMesh();

	for ( const FName& BoneName : SimulatedMeshBones )
	{
		MeshComponent->SetBodySimulatePhysics( BoneName, bSimulate );
		MeshComponent->SetEnableBodyGravity( bSimulate, BoneName );
	}

	for ( const FName& BoneName : SimulatedMeshBonesBelow )
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
	BulbMeshComponent->SetHiddenInGame( false );
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
	BulbMeshComponent->SetHiddenInGame( true );
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
	const FVector& HitLocation,
	const FVector& KnockbackDirection,
	const float DistanceFromAttacker
)
{
	const bool bIsAlive = HealthComponent->IsAlive();

	// Force un-faking death
	if ( bIsAlive && State == EZeroEnemyState::FakingDeath )
	{
		UnFakeDeath();
	}

	if ( USkeletalMeshComponent* SkeletalMesh = GetMesh() )
	{
		const FString BoneNameString = BoneName.ToString();

		const bool bLegBone = BoneNameString.Contains( "leg" );
		if ( !bLegBone )
		{
			const bool bBoneKnockable = BoneNameString.Contains( "neck" )
				|| ( BoneNameString.Contains( "spine" ) && DistanceFromAttacker < MaxKnockOutDistance );
			if ( bIsAlive && bBoneKnockable )
			{
				KnockOut();

				const float KnockbackForce = Data->KnockOutKnockbackCurve->GetFloatValue( DistanceFromAttacker ) * 1.0f;
				ApplyKnockback( KnockbackDirection, KnockbackForce );
			}

			#ifdef UE_WITH_CHEAT_MANAGER
			if ( UConvarLibrary::IsGunDebugEnabled() )
			{
				UUtilityLibrary::PrintWarning(
					TEXT( "ZeroEnemy: Hit bone %s instead of leg" ),
					*BoneNameString
				);
			}
			#endif
			return false;
		}

		// Destroy bone only once
		if ( !SkeletalMesh->IsBoneHiddenByName( BoneName ) )
		{
			SkeletalMesh->HideBoneByName( BoneName, EPhysBodyOp::PBO_None );

			// NOTE: Using SetCollisionEnabled doesn't seem to work so we use this one instead.
			SkeletalMesh->GetBodyInstance( BoneName )->SetShapeCollisionEnabled(
				0,
				ECollisionEnabled::NoCollision
			);

			if ( bIsAlive )
			{
				KnockOut();
			}
		}

		const bool bBackLeg = BoneNameString.Contains( "back" );
		const bool bRightSide = BoneNameString.Contains( "_r" );

		// Computing appropriate leg index depending on whenever it is in back/front and left/right sides.
		// See DeadLegs comment.
		const int32 LegIndex = static_cast<int32>( bBackLeg ) * 2 + static_cast<int32>( bRightSide );
		checkf(
			LegIndex >= 0 && LegIndex < MAX_NUM_ZERO_ENEMY_LEGS,
			TEXT( "Leg index is out-of-range with value %d" ),
			LegIndex
		);

		#ifdef UE_WITH_CHEAT_MANAGER
		if ( UConvarLibrary::IsGunDebugEnabled() )
		{
			UUtilityLibrary::PrintMessage(
				TEXT( "ZeroEnemy: Hit bone %s as leg index %d" ),
				*BoneNameString, LegIndex
			);
		}
		#endif

		// Check whether the leg is alive
		// Returning early to prevent body parts update
		if ( DeadLegs[LegIndex] ) return false;
		
		DeadLegs[LegIndex] = true;
	}
	else
	{
		BodyPart->DestroyComponent();
	}

	LeftBodyPartsCount -= 1;
	if ( LeftBodyPartsCount <= Data->BodyPartsLeftToKill ) return true;

	if ( bIsAlive )
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

	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if ( SkeletalMesh->IsSimulatingPhysics() )
	{
		SkeletalMesh->SetAllPhysicsLinearVelocity( Direction * Force );
		return;
	}

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
		LightManagerComponent->FlickerLightsInRadius(
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

bool AZeroEnemy::TakeDamage_Implementation( FDamageContext& DamageContext )
{
	UPrimitiveComponent* HitComponent = DamageContext.HitResult.GetComponent();

	const FVector KnockbackDirection = UKismetMathLibrary::GetDirectionUnitVector(
		DamageContext.HitResult.TraceStart,
		DamageContext.HitResult.TraceEnd
	);

	// Dispatch event that we have been attacked
	// It will be used by the AI controller to auto-target the attacker
	OnAttacked.Broadcast( DamageContext.AttackerActor );

	// Deals damage with player's stomp when on the ground
	if ( GetMesh()->IsSimulatingPhysics() && DamageContext.DamageType == EDamageType::Melee )
	{
		ApplyKnockback( KnockbackDirection, Data->MeleeKnockbackForce );
		return true;
	}

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

			// Force kill this actor
			DamageContext.DamageAmount = HealthComponent->CurrentHealth;
			return true;
		}

		return false;
	}

	// Check if damaged one of its body part
	if ( HitComponent == GetMesh() )
	{
		const float DistanceFromAttacker = IsValid( DamageContext.AttackerActor )
			? FVector::Distance( DamageContext.AttackerActor->GetActorLocation(), GetActorLocation() )
			: 0.0f;

		const bool bIsDead = DestroyBodyPart(
			HitComponent,
			DamageContext.HitResult.BoneName,
			DamageContext.HitResult.Location,
			KnockbackDirection,
			DistanceFromAttacker
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

bool AZeroEnemy::IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType )
{
	return HealthComponent->IsAlive() && !GetMesh()->IsSimulatingPhysics();
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
	Data->KnockOutKnockbackCurve->GetTimeRange( Temp, MaxKnockOutDistance );
	
	// TODO: Implement AimAssist on legs
	StartBodyPartsCount = LeftBodyPartsCount;
	ensureAlwaysMsgf(
		StartBodyPartsCount > 0,
		TEXT( "ZeroEnemy %s doesn't have any body parts count!" ),
		*GetName()
	);
	
	/*auto BodyParts = UUtilityLibrary::GetComponentsOfActorByTag<UMeshComponent>(
		this,
		Data->BodyPartTag
	);
	StartBodyPartsCount = BodyParts.Num();
	LeftBodyPartsCount = StartBodyPartsCount;*/

	// Enable aim assist for all body parts
	/*for ( auto BodyPart : BodyParts )
	{
		if ( Data->bBodyPartHasAimAssist )
		{
			BodyPart->SetCollisionObjectType( Data->AimAssistCollisionChannel );
		}

		BodyPart->SetCollisionResponseToChannels( Data->BodyPartDefaultCollisions );
	}*/

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
	if ( !HealthComponent->IsAlive() ) return;

	if ( State == EZeroEnemyState::FakingDeath )
	{
		UnFakeDeath();
	}
}

void AZeroEnemy::OnDeath( const FDamageContext& DamageContext )
{
	CloseBulb();

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.ClearTimer( StunTimerHandle );
	TimerManager.ClearTimer( RushTimerHandle );
	TimerManager.ClearTimer( KnockOutTimerHandle );

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
	if ( DamageContext.DamageType != EDamageType::Melee && !Direction.IsNearlyZero() )
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
