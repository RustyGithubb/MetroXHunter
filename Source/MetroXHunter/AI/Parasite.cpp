/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/Parasite.h"
#include "AI/PossessableCorpse.h"
#include "AI/ZeroEnemy.h"

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
}

void AParasite::BeginPlay()
{
	UpdateDataAsset();

	OnActorHit.AddDynamic( this, &AParasite::OnHit );
	HealthComponent->OnDeath.AddDynamic( this, &AParasite::OnDeath );

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
	}
}

void AParasite::UpdateDataAsset()
{
	verifyf( IsValid( DataAsset ), TEXT( "%s doesn't reference a DataAsset" ), *GetName() );

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->MaxWalkSpeed = DataAsset->WalkSpeed;
	MovementComponent->RotationRate.Yaw = DataAsset->YawRotationRate;

	HealthComponent->MaxHealth = DataAsset->MaxHealth;
}

void AParasite::PossessCorpse( APossessableCorpse* Corpse )
{
	verify( IsValid( Corpse ) );

	// Destroy corpse and parasite
	Corpse->Destroy();
	Destroy();

	// Set to always spawn to avoid annoying crashes because of collisions at spawn
	FActorSpawnParameters SpawnParams {};
	SpawnParams.SpawnCollisionHandlingOverride = 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Spawn enemy
	auto Enemy = GetWorld()->SpawnActor<AZeroEnemy>( 
		Corpse->EnemyClass,
		// TODO: Export location offset
		Corpse->GetActorLocation() + FVector { 0.0f, 0.0f, 180.0f * 0.5f },
		Corpse->GetActorRotation(),
		SpawnParams
	);
	verifyf( IsValid( Enemy ), TEXT( "Failed to spawn enemy during parasite possession!" ) );
	Enemy->Data = Corpse->DataAsset;
	Enemy->Stun( 2.0f, false );

	OnPossessCorpse.Broadcast( this, Corpse, Enemy );
}

//void AParasite::EnterVent( AVent* Vent )
//{
//	verify( IsValid( Vent ) );
//	Vent->EnterVent( this );
//}

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

void AParasite::OnHit(
	AActor* SelfActor, AActor* OtherActor,
	FVector NormalImpulse,
	const FHitResult& Hit
)
{
	if ( !bIsJumpAttacking ) return;

	// Prevent damaging himself
	if ( !IsValid( OtherActor ) || OtherActor == SelfActor ) return;

	auto HitHealthComponent = OtherActor->GetComponentByClass<UHealthComponent>();
	if ( !IsValid( HitHealthComponent ) ) return;

	FDamageContext DamageContext {};
	DamageContext.AttackerActor = this;
	DamageContext.DamageAmount = DataAsset->JumpAttackDamage;
	DamageContext.DamageType = EDamageType::Melee;
	DamageContext.HitResult = Hit;

	HitHealthComponent->TakeDamage( DamageContext );
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

	// Schedule blood spawn only if not dead by electricity
	if ( DamageContext.DamageType != EDamageType::Shock )
	{
		MeshComponent->OnComponentHit.AddDynamic( this, &AParasite::OnRagdollMeshHit );
	}

	// Apply knockback to mesh
	const FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(
		DamageContext.HitResult.TraceStart,
		DamageContext.HitResult.TraceEnd
	);
	if ( !Direction.IsNearlyZero() )
	{
		const FVector Knockback = Direction * 500.0f + FVector::UpVector * -300.0f;
		//MeshComponent->AddImpulseAtLocation( Knockback, DamageContext.HitResult.ImpactPoint );
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
		DataAsset->BloodPuddleScale
	);
}
