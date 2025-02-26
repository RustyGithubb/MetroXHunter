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
#include "Health/HealthComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/CapsuleComponent.h"

#include "Library/UtilityLibrary.h"
#include "Library/GameplayLibrary.h"

#include "Kismet/KismetMathLibrary.h"

AParasite::AParasite()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "HealthComponent" ) );
	HealthComponent->MaxHealth = 1;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>( TEXT( "PawnSensingComponent" ) );

	SaveComponent = CreateDefaultSubobject<USaveLoadComponent>( TEXT( "SaveComponent" ) );
}

void AParasite::BeginPlay()
{
	UpdateDataAsset();

	OnActorHit.AddDynamic( this, &AParasite::OnHit );
	HealthComponent->OnDeath.AddDynamic( this, &AParasite::OnDeath );

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

bool AParasite::IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType )
{
	return HealthComponent->IsAlive();
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
	Enemy->Stun( 2.0f, false );

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

	if ( auto OtherPawn = Cast<APawn>( OtherActor ) )
	{
		// Don't damage pawns that are not the player
		if ( !OtherPawn->IsPlayerControlled() ) return;
	}

	UHealthComponent* HitHealthComponent = OtherActor->GetComponentByClass<UHealthComponent>();
	if ( !IsValid( HitHealthComponent ) ) return;

	if ( CinematicMode == EParasiteCinematicMode::RushPlayer )
	{
		DoBiteAttack( OtherActor );

		//	Remove cinematic mode so we're not rushed twice by the same enemy
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

	// Simulate physics and setup collisions on body mesh
	USkeletalMeshComponent* MeshComponent = GetMesh();
	MeshComponent->SetSimulatePhysics( true );
	MeshComponent->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
	MeshComponent->SetCollisionResponseToChannels( DataAsset->MeshRagdollCollisions );
	MeshComponent->SetReceivesDecals( false );

	// Apply knockback to mesh
	const FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(
		DamageContext.HitResult.TraceStart,
		DamageContext.HitResult.TraceEnd
	);
	if ( !Direction.IsNearlyZero() )
	{
		const FVector Knockback = Direction * 500.0f + FVector::UpVector * -100.0f;
		MeshComponent->SetAllPhysicsLinearVelocity( Knockback, true );
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
