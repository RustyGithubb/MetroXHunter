/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/Parasite.h"
#include "AI/ParasiteAIController.h"
#include "AI/PossessableCorpse.h"
#include "AI/ZeroEnemy.h"
#include "AI/ZeroEnemyAIController.h"

#include "Sounds/ParasiteSoundManagerComponent.h"

#include "Vent/Vent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/CapsuleComponent.h"

#include "Library/UtilityLibrary.h"
#include "Library/GameplayLibrary.h"
#include "Library/ConvarLibrary.h"

#include "Kismet/KismetMathLibrary.h"

AParasite::AParasite()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "HealthComponent" ) );
	HealthComponent->MaxHealth = 1;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>( TEXT( "PawnSensingComponent" ) );

	ElectrocutableComponent = CreateDefaultSubobject<UElectrocutableComponent>( TEXT( "ElectrocutableComponent" ) );

	SaveComponent = CreateDefaultSubobject<USaveLoadComponent>( TEXT( "SaveComponent" ) );
}

void AParasite::BeginPlay()
{
	DefaultMeshCollisions = GetMesh()->GetCollisionResponseToChannels();
	DefaultMeshRelativeTransform = GetMesh()->GetRelativeTransform();

	UpdateDataAsset();

	OnActorHit.AddDynamic( this, &AParasite::OnHit );
	HealthComponent->OnDeath.AddDynamic( this, &AParasite::OnDeath );
	
	ElectrocutableComponent->OnElectricStart.AddDynamic( this, &AParasite::OnElectricStart );
	ElectrocutableComponent->OnElectricEnd.AddDynamic( this, &AParasite::OnElectricEnd );

	SoundManagerComponent = GetComponentByClass<UParasiteSoundManagerComponent>();

	Super::BeginPlay();
}

void AParasite::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AParasite::Landed( const FHitResult& Hit )
{
	Super::Landed( Hit );

	//	Reset jump attack
	if ( bIsJumpAttacking )
	{
		bIsJumpAttacking = false;
		bHasAlreadyDamaged = false;
	}
}

bool AParasite::TakeDamage_Implementation( UPARAM( ref ) FDamageContext& DamageContext )
{
	// Acid should one-shot Parasites, no matter their damage.
	if ( DamageContext.DamageType == EDamageType::Spit )
	{
		DamageContext.DamageAmount = HealthComponent->CurrentHealth;
	}

	const bool bIsDeathBlow = DamageContext.DamageAmount >= DamageContext.HealthComponent->CurrentHealth;
	if ( bIsDeathBlow )
	{
		// Emit blood splash only if not dead by electricity
		if ( !HasEmittedBlood() && DamageContext.DamageType != EDamageType::Shock )
		{
			EmitBloodSplash( DamageContext );
		}

		// Apply impulse to mesh
		if ( !DamageContext.HealthComponent->IsAlive() )
		{
			const FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(
				DamageContext.HitResult.TraceStart,
				DamageContext.HitResult.TraceEnd
			);
			if ( !Direction.IsNearlyZero() )
			{
				GetMesh()->SetAllPhysicsLinearVelocity( Direction * 500.0f, true );
			}
		}
	}

	return true;
}

bool AParasite::IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType, const FHitResult& HitResult )
{
	if ( !HealthComponent->IsAlive() ) return false;
	if ( GetMesh()->IsSimulatingPhysics() ) return false;

	return true;
}

FVector AParasite::GetEQSStartLocation_Implementation() const
{
	const AController* SelfController = GetController();
	return IEQSContextProvider::Execute_GetEQSStartLocation( SelfController );
}

AActor* AParasite::GetEQSTargetActor_Implementation() const
{
	const AController* SelfController = GetController();
	return IEQSContextProvider::Execute_GetEQSTargetActor( SelfController );
}

void AParasite::UpdateDataAsset()
{
	verifyf( IsValid( DataAsset ), TEXT( "%s doesn't reference a DataAsset" ), *GetName() );

	// Randomize scale
	RandomMeshScale = UUtilityLibrary::RandomInRange( DataAsset->ScaleRange );
	const FVector Scale = FVector( RandomMeshScale );
	GetMesh()->SetRelativeScale3D( Scale );

	// Randomize movement speed
	const float MovementSpeedScale = FMath::GetMappedRangeValueUnclamped(
		FVector2f( DataAsset->ScaleRange.GetLowerBoundValue(), DataAsset->ScaleRange.GetUpperBoundValue() ),
		FVector2f( DataAsset->MovementSpeedScaleRange.GetUpperBoundValue(), DataAsset->MovementSpeedScaleRange.GetLowerBoundValue() ),
		static_cast<float>( Scale.X )
	);

	// Store move speed values
	DefaultMoveSpeed = DataAsset->WalkSpeed * MovementSpeedScale;
	FleeMoveSpeed = DefaultMoveSpeed * DataAsset->FleeSpeedScale;

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->MaxWalkSpeed = DefaultMoveSpeed;
	MovementComponent->RotationRate.Yaw = DataAsset->YawRotationRate;

	HealthComponent->MaxHealth = DataAsset->MaxHealth;
}

void AParasite::PossessCorpse( APossessableCorpse* Corpse )
{
	verify( IsValid( Corpse ) );

	// Set to always spawn to avoid annoying crashes because of collisions at spawn
	FActorSpawnParameters SpawnParams {};
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn enemy
	AZeroEnemy* Enemy = GetWorld()->SpawnActor<AZeroEnemy>(
		Corpse->EnemyClass,
		// TODO: Export location offset
		Corpse->GetActorLocation() + FVector { 0.0f, 0.0f, 180.0f * 0.5f },
		Corpse->GetActorRotation(),
		SpawnParams
	);
	verifyf( IsValid( Enemy ), TEXT( "Failed to spawn enemy during parasite possession!" ) );
	Enemy->Data = Corpse->DataAsset;
	Enemy->Stun( 2.0f );

	AParasiteAIController* SelfAIController = GetController<AParasiteAIController>();
	AZeroEnemyAIController* EnemyAIController = Enemy->GetController<AZeroEnemyAIController>();
	if ( SelfAIController != nullptr && EnemyAIController != nullptr )
	{
		// Transfer current target to the new enemy
		AActor* Target = SelfAIController->GetEnemy();
		EnemyAIController->SetTarget( Target );
	}

	OnPossessCorpse.Broadcast( this, Corpse, Enemy );

	// Destroy corpse and parasite
	// NOTE: It is important to destroy these as late as possible within the function
	Corpse->Destroy();
	Destroy();
}

void AParasite::PrepareJumpAttack_Implementation()
{
	bIsPreparingJump = true;

	// Rest of the code like play sound, animation and warts blinking are in the blueprint.
}

void AParasite::EndPrepareJumpAttack_Implementation()
{
	bIsPreparingJump = false;
}

bool AParasite::IsPreparingJump() const
{
	return bIsPreparingJump;
}

void AParasite::SetStompLocked( bool bIsLocked )
{
	bIsStompLocked = bIsLocked;

	if ( bIsStompLocked )
	{
		GetCharacterMovement()->StopMovementImmediately();
	}
}

bool AParasite::IsStompLocked() const
{
	return bIsStompLocked;
}

void AParasite::JumpAttack()
{
	FVector UpDirection = FVector::UpVector;
	FVector ForwardDirection = GetActorForwardVector();
	const FVector JumpDirection = FVector::SlerpNormals( UpDirection, ForwardDirection, 0.75 );

	LaunchCharacter(
		JumpDirection * 1000.0f,
		/* bXYOverride */ false, /* bZOverride */ false
	);

	bIsJumpAttacking = true;
}

bool AParasite::IsJumpAttacking() const
{
	return bIsJumpAttacking;
}

void AParasite::EmitBloodSplash_Implementation( const FDamageContext& DamageContext )
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->OnComponentHit.AddDynamic( this, &AParasite::OnRagdollMeshHit );

	bHasEmittedBlood = true;
}

bool AParasite::HasEmittedBlood() const
{
	return bHasEmittedBlood;
}

float AParasite::GetDefaultMoveSpeed() const
{
	return DefaultMoveSpeed;
}

float AParasite::GetFleeMoveSpeed() const
{
	return FleeMoveSpeed;
}

float AParasite::GetRandomMeshScale() const
{
	return RandomMeshScale;
}

void AParasite::OnHit(
	AActor* SelfActor, AActor* OtherActor,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if ( !bIsJumpAttacking || bHasAlreadyDamaged ) return;

	// Prevent damaging himself
	if ( !IsValid( OtherActor ) || OtherActor == SelfActor ) return;

	const APawn* OtherPawn = Cast<APawn>( OtherActor );
	if ( OtherPawn != nullptr )
	{
		// Don't damage pawns that are not the player
		if ( !OtherPawn->IsPlayerControlled() ) return;
	}

	UHealthComponent* HitHealthComponent = OtherActor->GetComponentByClass<UHealthComponent>();
	if ( !IsValid( HitHealthComponent ) ) return;

	bool bShouldLaunchQTE = false;

	// Avoid QTE on non-pawns actors
	if ( OtherPawn != nullptr )
	{
		bShouldLaunchQTE = CinematicMode == EParasiteCinematicMode::RushPlayer;

		if ( !bShouldLaunchQTE )
		{
			// QTE when health of the player is too low
			const float ThresholdRatio = DataAsset->RushAttackQuickTimeEventHealthThreshold / 100.0f;
			bShouldLaunchQTE = HitHealthComponent->GetHealthRatio() <= ThresholdRatio;
		}
		if ( !bShouldLaunchQTE )
		{
			// QTE when attacking from behind
			const FVector SelfForward = GetActorForwardVector();
			const FVector OtherForward = OtherActor->GetActorForwardVector();
			bShouldLaunchQTE = FVector::DotProduct( SelfForward, OtherForward ) > 0.1f;
		}
	}

	// Prevent QTE when this convar is enabled
	if ( UConvarLibrary::IsAINoQTEConvarEnabled() )
	{
		bShouldLaunchQTE = false;
	}

	if ( bShouldLaunchQTE )
	{
		DoBiteAttack( OtherActor );

		// Remove cinematic mode so we're not rushed twice by the same enemy
		CinematicMode = EParasiteCinematicMode::None;
	}
	else
	{
		FDamageContext DamageContext {};
		DamageContext.AttackerActor = this;
		DamageContext.DamageAmount = DataAsset->JumpAttackDamage;
		DamageContext.DamageType = EDamageType::Melee;
		DamageContext.HitResult = Hit;

		HitHealthComponent->TakeDamage( DamageContext );
	}


	bHasAlreadyDamaged = true;
}

void AParasite::OnDeath( const FDamageContext& DamageContext )
{
	if ( IsValid( Controller ) )
	{
		Controller->Destroy();
	}

	SimulateRagdoll();

	// Apply knockback to mesh
	const FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(
		DamageContext.HitResult.TraceStart,
		DamageContext.HitResult.TraceEnd
	);
	if ( !Direction.IsNearlyZero() )
	{
		const FVector Knockback = Direction * 500.0f + FVector::UpVector * -100.0f;
		GetMesh()->SetAllPhysicsLinearVelocity( Knockback, true );
	}

	// Disable capsule component's collisions
	GetCapsuleComponent()->SetCollisionEnabled( ECollisionEnabled::NoCollision );
}

void AParasite::OnRagdollMeshHit(
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
	if ( VelocityLength > DataAsset->BloodPuddleSpawnMaxVelocity ) return;

	SpawnBloodPuddle();

	GetMesh()->OnComponentHit.RemoveDynamic( this, &AParasite::OnRagdollMeshHit );
}

void AParasite::OnElectricStart( float Duration )
{
	SimulateRagdoll();

	OnStunned.Broadcast( true );
}

void AParasite::OnElectricEnd()
{
	if ( HealthComponent->IsAlive() )
	{
		UnSimulateRagdoll();
	}

	OnStunned.Broadcast( false );
}

void AParasite::SpawnBloodPuddle()
{
	UGameplayLibrary::SpawnBloodPuddleAtBone(
		this,
		DataAsset->BloodPuddleClass,
		GetMesh(),
		DataAsset->BloodPuddleSpawnBoneName,
		GetActorScale3D() * DataAsset->BloodPuddleScale
	);
}

void AParasite::SimulateRagdoll()
{
	// Simulate physics and setup collisions on body mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( true );
	MeshComponent->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
	MeshComponent->SetCollisionResponseToChannels( DataAsset->MeshRagdollCollisions );
}

void AParasite::UnSimulateRagdoll()
{
	ResolveLocationFromRagdoll();

	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( false );
	MeshComponent->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	MeshComponent->SetCollisionResponseToChannels( DefaultMeshCollisions );

	const FTransform NewRelativeTransform {
		DefaultMeshRelativeTransform.GetRotation(),
		DefaultMeshRelativeTransform.GetLocation(),
		FVector( RandomMeshScale )
	};
	MeshComponent->SetRelativeTransform( NewRelativeTransform );
}

void AParasite::ResolveLocationFromRagdoll()
{
	const USkeletalMeshComponent* MeshComponent = GetMesh();
	const FVector PelvisLocation = MeshComponent->GetSocketLocation( NAME_None );

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
