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
#include "Sounds/ZeroEnemySoundManagerComponent.h"

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
		case EZeroEnemyState::UnRagdoll:
		{
			const FTimerManager& TimerManager = GetWorldTimerManager();
			const float ElapsedTime = TimerManager.GetTimerElapsed( UnRagdollTimerHandle );
			const float RemainingTime = TimerManager.GetTimerRemaining( UnRagdollTimerHandle );
			const float Alpha = ElapsedTime / ( ElapsedTime + RemainingTime );

			const FTransform ActorToWorld = GetTransform();

			USkeletalMeshComponent* MeshComponent = GetMesh();
			MeshComponent->SetWorldLocationAndRotation(
				FMath::Lerp(
					LastWorldRagdollLocation, 
					ActorToWorld.TransformPosition( DefaultMeshRelativeTransform.GetLocation() ),
					Alpha
				),
				FQuat::Slerp(
					LastWorldRagdollRotation.Quaternion(),
					ActorToWorld.TransformRotation( DefaultMeshRelativeTransform.GetRotation() ),
					Alpha
				)
			);
			break;
		}
		case EZeroEnemyState::RushAttack:
		{
			// Get current rush time
			const FTimerManager& TimerManager = GetWorld()->GetTimerManager();
			const float CurrentRushTime = TimerManager.GetTimerElapsed( RushTimerHandle );

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

void AZeroEnemy::FellOutOfWorld( const UDamageType& DamageType )
{
	HealthComponent->OnDeath.Broadcast( FDamageContext {} );

	Super::FellOutOfWorld( DamageType );
}

void AZeroEnemy::FakeDeath_Implementation()
{
	SetState( EZeroEnemyState::FakingDeath );

	Ragdoll();
}

void AZeroEnemy::UnFakeDeath_Implementation()
{
	ensureMsgf(
		State == EZeroEnemyState::FakingDeath,
		TEXT( "ZeroEnemy: %s is supposed to be in the FakingDeath state" ),
		*GetName()
	);

	UnRagdoll( false );
}

void AZeroEnemy::KnockOut( float RagdollTime )
{
	if ( RagdollTime <= 0.0f )
	{
		RagdollTime = 5.0f;
	}

	// TODO: I may need a real state machine at this point.
	if ( State == EZeroEnemyState::RushAttack )
	{
		StopRushAttack();
	}

	if ( State != EZeroEnemyState::KnockOut )
	{
		// Force stop animations
		GetMesh()->GetAnimInstance()->StopAllMontages( 0.0f );

		// NOTE: Workaround to avoid crash when shooting weak points of the creature, probably due to animation still playing
		//		 while simulating physics?
		GetWorldTimerManager().SetTimerForNextTick(
			[&]()
			{
				GetWorldTimerManager().SetTimerForNextTick( this, &AZeroEnemy::Ragdoll );
			}
		);

		SoundManagerComponent->OnKnockedOut();
		SetState( EZeroEnemyState::KnockOut );
	}

	GetWorldTimerManager().SetTimer( KnockOutTimerHandle, this, &AZeroEnemy::UnKnockOut, RagdollTime );
}

void AZeroEnemy::UnKnockOut()
{
	UnRagdoll();
	GetWorldTimerManager().ClearTimer( KnockOutTimerHandle );
}

void AZeroEnemy::Ragdoll()
{
	// Ragdoll mesh
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if ( SkeletalMesh->IsSimulatingPhysics() ) return;

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();

	SkeletalMesh->SetSimulatePhysics( true );
	SkeletalMesh->SetCollisionResponseToChannels( Data->MeshRagdollCollisions );

	// Disable tick and character movement
	//SetActorTickEnabled( false );
	CharacterMovementComponent->SetActive( false );

	SetCollisionsEnabled( false );
}

void AZeroEnemy::UnRagdoll( bool bBlendOverTime )
{
	// Un-ragdoll mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if ( !MeshComponent->IsSimulatingPhysics() ) return;

	if ( bBlendOverTime )
	{
		MeshComponent->GetAnimInstance()->SavePoseSnapshot( TEXT( "Ragdoll" ) );

		LastWorldRagdollLocation = MeshComponent->GetSocketLocation( NAME_None );
		LastWorldRagdollRotation = MeshComponent->GetSocketRotation( NAME_None );
	}

	ResolveLocationFromRagdoll();

	MeshComponent->bPauseAnims = false;

	MeshComponent->SetSimulatePhysics( false );
	// NOTE: We must re-attach the mesh to the root component because simulating physics
	// de-attach components from their parent.
	MeshComponent->AttachToComponent( RootComponent, FAttachmentTransformRules::KeepRelativeTransform );
	MeshComponent->SetCollisionResponseToChannels( DefaultMeshCollisions );

	SetCollisionsEnabled( true );
	//SetActorTickEnabled( true );

	if ( bBlendOverTime )
	{
		SoundManagerComponent->OnRevival();

		// Unragdoll for a small time
		SetState( EZeroEnemyState::UnRagdoll );
		GetWorldTimerManager().SetTimer( UnRagdollTimerHandle, this, &AZeroEnemy::EndRagdollState, 1.0f );
	}
	else
	{
		EndRagdollState();
	}
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
	ECollisionEnabled::Type BulbCollisionType = bEnabled
		? ECollisionEnabled::QueryAndPhysics
		: ECollisionEnabled::QueryOnly;

	GetCapsuleComponent()->SetCollisionEnabled( CollisionType );
	BulbMeshComponent->SetCollisionEnabled( BulbCollisionType );
}

void AZeroEnemy::OpenBulb( float OpenTime )
{
	BulbMeshComponent->SetMaterial( 0, Data->OpenedBulbMaterial );

	// Enable collisions
	BulbMeshComponent->SetCollisionEnabled( ECollisionEnabled::QueryOnly );

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

	// Disable collisions
	BulbMeshComponent->SetCollisionEnabled( ECollisionEnabled::NoCollision );

	// Clear potential on-going timer
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer( OpeningBulbTimerHandle );

	bIsBulbOpened = false;

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Close Bulb" ) );
}

void AZeroEnemy::Stun( float StunTime )
{
	SetState( EZeroEnemyState::Stun );

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(
		StunTimerHandle,
		this, &AZeroEnemy::UnStun,
		StunTime
	);

	OnStun.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Start Stun" ) );
}

void AZeroEnemy::UnStun()
{
	if ( State == EZeroEnemyState::Stun )
	{
		SetState( EZeroEnemyState::None );
	}

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

	if ( !GetMesh()->IsSimulatingPhysics() )
	{
		Stun( Data->PanicStunTime );
	}
}

int32 AZeroEnemy::GetStartingBodyPartsCount() const
{
	return StartBodyPartsCount;
}

void AZeroEnemy::ApplyKnockback( const FVector& Direction, float Force )
{
	if ( Force == 0.0f ) return;
	if ( State == EZeroEnemyState::UnFakingDeath ) return;

	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if ( SkeletalMesh->IsSimulatingPhysics() )
	{
		const FVector Impulse = Direction * Force;
		SkeletalMesh->SetAllPhysicsLinearVelocity( Impulse );
		UE_LOG( LogTemp, Log, TEXT( "ApplyKnockback: set physics linear velocity %s" ), *Impulse.ToString() );
		return;
	}

	FVector Impulse = Direction.GetSafeNormal2D() * Force;
	Impulse.Z = Data->DefaultKnockbackZ;

	GetCharacterMovement()->AddImpulse( Impulse, true );

	UE_LOG( LogTemp, Log, TEXT( "ApplyKnockback: add impulse %s" ), *Impulse.ToString() );
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

	SoundManagerComponent->OnRushScream();

	OnRush.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Start Rush Attack" ) );
}

void AZeroEnemy::StartResolveRushAttack_Implementation()
{
	SetState( EZeroEnemyState::RushAttackResolve );

	GetWorld()->GetTimerManager().ClearTimer( RushTimerHandle );

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Start resolving Rush Attack" ) );
}

void AZeroEnemy::StopRushAttack_Implementation()
{
	SetState( EZeroEnemyState::None );

	GetWorld()->GetTimerManager().ClearTimer( RushTimerHandle );

	// Reset walk speed
	UpdateWalkSpeed();
	GetCharacterMovement()->RotationRate.Yaw = Data->YawRotationRate;

	OnUnRush.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "Stop Rush Attack" ) );
}

void AZeroEnemy::SpitAttack_Implementation( const FVector& TargetLocation )
{
	auto SpitProjectile = GetWorld()->SpawnActor<ASpitProjectile>(
		Data->SpitProjectileClass,
		GetSpitAttackOrigin(), GetActorRotation()
	);
	if ( !IsValid( SpitProjectile ) ) return;

	SpitProjectile->SetOwner( this );
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
	//UGameplayStatics::PlaySoundAtLocation(
	//	this,
	//	Data->ScreamSound,
	//	GetActorLocation(),
	//	/* VolumeMultiplier */ 1.0f,
	//	/* PitchMultiplier */ 1.0f,
	//	/* StartTime */ 0.0f,
	//	Data->SoundAttenuation,
	//	Data->SoundConcurrency
	//);

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

bool AZeroEnemy::AreClawsDestroyed() const
{
	return bDeadClaws[0] && bDeadClaws[0] == bDeadClaws[1];
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

	switch ( GetState() )
	{
		// Only wake-up when damaged
		case EZeroEnemyState::FakingDeath:
			UnFakeDeath();
			break;
	}

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
			SoundManagerComponent->OnBulbDestroy( DamageContext.HitResult );

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

		float KnockbackForce = Data->BodyPartHitKnockbackForce;
		const bool bIsDead = DestroyBodyPart(
			HitComponent,
			DamageContext.HitResult.BoneName,
			DamageContext.HitResult.Location,
			DistanceFromAttacker,
			KnockbackForce
		);
		if ( bIsDead )
		{
			// Force kill this actor
			DamageContext.DamageAmount = HealthComponent->CurrentHealth;
			return true;
		}

		ApplyKnockback( KnockbackDirection, KnockbackForce );
		return false;
	}

	// Knockback for all damage except electricity
	if ( DamageContext.DamageType != EDamageType::Shock )
	{
		ApplyKnockback( KnockbackDirection, Data->WholeBodyHitKnockbackForce );
	}

	return false;
}

bool AZeroEnemy::IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType, const FHitResult& HitResult )
{
	if ( HealthComponent->bIsInvulnerable ) return false;
	if ( !HealthComponent->IsAlive() ) return false;

	// We shouldn't give hints that it's dangerous...
	if ( GetState() == EZeroEnemyState::FakingDeath ) return false;

	if ( HitResult.Component == BulbMeshComponent ) return IsBulbOpened();

	if ( GetMesh()->IsSimulatingPhysics() )
	{
		return HitResult.Component == GetMesh() && HitResult.BoneName.ToString().Contains( "leg" );
	}

	return true;
}

bool AZeroEnemy::CanCallTakeDamage_Implementation( const FDamageContext& DamageContext )
{
	return !HealthComponent->bIsInvulnerable;
}

bool AZeroEnemy::DestroyBodyPart(
	USceneComponent* BodyPart,
	const FName& BoneName,
	const FVector& HitLocation,
	const float DistanceFromAttacker,
	float& KnockbackForce
)
{
	const bool bIsAlive = HealthComponent->IsAlive();

	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	const FString BoneNameString = BoneName.ToString();

	const bool bLegBone = BoneNameString.Contains( "leg" );
	if ( !bLegBone )
	{
		// Maybe an enum flag with bitwise operations would be much more pratical and "optimized" instead
		// of checking everytime all bone types. Or maybe a look-up table (a.k.a. set)?
		// But whatever, it works good-enough right?
		const bool bIsClawBone = BoneNameString.Contains( "claw" );
		const bool bIsHeadBone = BoneNameString.Contains( "head" );
		const bool bIsSpineBone = BoneNameString.Contains( "spine" );
		const bool bIsBodyBone = bIsSpineBone || bIsHeadBone
			|| BoneNameString.Contains( "arm" ) || BoneNameString.Contains( "clavicle" )
			|| BoneNameString.Contains( "thigh" ) || BoneNameString.Contains( "calf" );
		if ( ( bIsClawBone || bIsBodyBone ) && !bIsSpineBone )
		{
			if ( bIsClawBone )
			{
				const bool bRightSide = BoneNameString.Contains( "_r" );
				const int32 ClawIndex = static_cast<int32>( bRightSide );
				bDeadClaws[ClawIndex] = true;

				// We force to destroy all claw bones instantly
				const FName ClawBoneName = bRightSide ? TEXT( "claw_1_r" ) : TEXT( "claw_1_l" );
				SetBoneHidden( ClawBoneName, /* bNewHidden */ true, /* bShouldTerminate */ true );
				OnLimbDestroyed( ClawBoneName );

				if ( AreClawsDestroyed() )
				{
					OpenBulb();
				}
			}
			else
			{
				SetBoneHidden( BoneName, /* bNewHidden */ true, /* bShouldTerminate */ true );
				OnLimbDestroyed( BoneName );
			}
			SoundManagerComponent->OnLimbDestroy( SkeletalMesh, BoneName, HitLocation );
		}

		const bool bBoneKnockable = bIsHeadBone || ( bIsBodyBone && DistanceFromAttacker < MaxKnockOutDistance );
		if ( bIsAlive && bBoneKnockable && !SkeletalMesh->IsSimulatingPhysics() )
		{
			KnockbackForce = Data->KnockOutKnockbackCurve->GetFloatValue( DistanceFromAttacker ) * 1.0f;
			return false;
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
		SetBoneHidden( BoneName, /* bNewHidden */ true, /* bShouldTerminate */ true );
		OnLimbDestroyed( BoneName );
		SoundManagerComponent->OnLegDestroy( SkeletalMesh, BoneName, HitLocation );
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
	if ( bDeadLegs[LegIndex] ) return false;
	bDeadLegs[LegIndex] = true;

	// Knock out when destroying a leg
	if ( bIsAlive && !SkeletalMesh->IsSimulatingPhysics() )
	{
		KnockOut();
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

FVector AZeroEnemy::GetEQSStartLocation_Implementation() const
{
	const AController* SelfController = GetController();
	return IEQSContextProvider::Execute_GetEQSStartLocation( SelfController );
}

AActor* AZeroEnemy::GetEQSTargetActor_Implementation() const
{
	const AController* SelfController = GetController();
	return IEQSContextProvider::Execute_GetEQSTargetActor( SelfController );
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

void AZeroEnemy::SetBoneHidden( FName BoneName, bool bNewHidden, bool bShouldTerminate )
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	UGameplayLibrary::SetBoneHidden( SkeletalMesh, BoneName, bNewHidden, bShouldTerminate );
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

	// Retrieve sound manager
	SoundManagerComponent = GetComponentByClass<UZeroEnemySoundManagerComponent>();
	checkf( IsValid( SoundManagerComponent ), TEXT( "Add a ZeroEnemySoundManagerComponent on the Gun class!" ) );
	
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

void AZeroEnemy::ResolveLocationFromRagdoll()
{
	const USkeletalMeshComponent* MeshComponent = GetMesh();
	const FVector PelvisLocation = MeshComponent->GetSocketLocation( "pelvis" );

	UCapsuleComponent* CollisionComponent = GetCapsuleComponent();
	const float CapsuleHalfHeight = CollisionComponent->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = CollisionComponent->GetScaledCapsuleRadius();

	FHitResult Hit {};
	bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(
		this,
		PelvisLocation + FVector::UpVector * CapsuleHalfHeight * 2.0f,
		PelvisLocation,
		CapsuleRadius, CapsuleHalfHeight,
		UEngineTypes::ConvertToTraceType( ECollisionChannel::ECC_Visibility ),
		/* bTraceComplex */ false,
		TArray<AActor*> {},
		UConvarLibrary::IsAIDebugConvarEnabled() ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hit,
		/* bIgnoreSelf */ true
	);

	// Simulated mesh is moving independently from the root component (i.e. CapsuleComponent)
	// so we have to re-locate the capsule before un-ragdolling.
	if ( bHit )
	{
		const FRotator NewRotation { 0.0f, CollisionComponent->GetComponentRotation().Yaw, 0.0f };
		CollisionComponent->SetWorldLocationAndRotation( Hit.Location, NewRotation );
	}
}

void AZeroEnemy::EndRagdollState()
{
	SetState( EZeroEnemyState::None );
	GetWorldTimerManager().ClearTimer( UnRagdollTimerHandle );

	// Safe-guard for getting up not lerping completely the location and rotation
	// due to premature state switching.
	GetMesh()->SetRelativeTransform( DefaultMeshRelativeTransform );

	SimulateMeshBonesPhysics( true );

	// Enable tick and character movement
	GetCharacterMovement()->SetActive( true );
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
