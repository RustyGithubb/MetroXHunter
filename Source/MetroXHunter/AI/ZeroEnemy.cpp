#include "AI/ZeroEnemy.h"
#include "AI/AISubstateManagerComponent.h"
#include "AI/AISubstate.h"

#include "AI/SpitProjectile.h"

#include "Health/HealthComponent.h"

#include "UtilityLibrary.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AZeroEnemy::AZeroEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	ProtoMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "ProtoMeshComponent" ) );
	ProtoMeshComponent->SetupAttachment( RootComponent );

	BulbMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BulbMeshComponent" ) );
	BulbMeshComponent->SetupAttachment( RootComponent );

	HealthComponent = CreateDefaultSubobject<UHealthComponent>( TEXT( "Health" ) );

	AISubstateManagerComponent = CreateDefaultSubobject<UAISubstateManagerComponent>(
		TEXT( "AISubstateManager" ) 
	);
}

void AZeroEnemy::BeginPlay()
{
	Super::BeginPlay();

	RetrieveReferences();
	
	InitializeAISubstateManager();
	UpdateWalkSpeed();

	GenerateBulb();
	CloseBulb();
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

	OpeningBulbTimerHandle.Invalidate();
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
	OpenBulb( Data->PanicBulbOpenTime );
	Stun( Data->PanicStunTime );
}

void AZeroEnemy::DestroyBodyPart( USceneComponent* BodyPart )
{
	BodyPart->DestroyComponent();

	LeftBodyPartsCount -= 1;

	if ( LeftBodyPartsCount <= Data->BodyPartsLeftToKill )
	{
		// TODO: Kill for real; here the implementation is only for BP
		bIsAlive = false;
		return;
	}

	MakePanic();
	UpdateWalkSpeed();
}

void AZeroEnemy::ApplyKnockback( const FVector& Direction, float Force )
{
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
	State = NewState;
	OnStateUpdate.Broadcast();
}

EZeroEnemyState AZeroEnemy::GetState() const
{
	return State;
}

float AZeroEnemy::GetMadnessLevel() const
{
	int32 SubstatesCount = AISubstateManagerComponent->GetSubstatesCount();
	if ( SubstatesCount == 0 ) return 0.0f;

	int32 SubstateIndex = AISubstateManagerComponent->GetSubstateIndex();
	return (float)SubstateIndex / (float)( SubstatesCount - 1 );
}

bool AZeroEnemy::TakeDamage_Implementation( const FDamageContext& DamageContext )
{
	UPrimitiveComponent* HitComponent = DamageContext.HitResult.GetComponent();
	
	// Check if damaged the bulb
	if ( HitComponent == BulbMeshComponent )
	{
		return IsBulbOpened();
	}

	const FVector KnockbackDirection = UKismetMathLibrary::GetDirectionUnitVector(
		DamageContext.HitResult.TraceStart,
		DamageContext.HitResult.TraceEnd
	);

	// Check if damaged one of its body part
	if ( IsValid( HitComponent ) && HitComponent->ComponentHasTag(Data->BodyPartTag) )
	{
		DestroyBodyPart( HitComponent );
		ApplyKnockback( KnockbackDirection, Data->BodyPartHitKnockbackForce );
		return false;
	}

	// Here, the body is hit

	ApplyKnockback( KnockbackDirection, Data->WholeBodyHitKnockbackForce );

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
		TEXT( "MadnessLevel" ),
		FString::SanitizeFloat( GetMadnessLevel() )
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

void AZeroEnemy::InitializeAISubstateManager()
{
	AISubstateManagerComponent->CreateSubstates( Data->SubstateClasses );
	AISubstateManagerComponent->SwitchToSubstate(
		SpawnSubstateClass != nullptr ? Data->SubstateClasses.Find( SpawnSubstateClass ) : 0
	);
}

void AZeroEnemy::GenerateBulb()
{
	auto BulbSpots = UUtilityLibrary::GetComponentsOfActorByTag<UArrowComponent>(
		this,
		Data->BulbSpotTag
	);

	auto PickedBulbSpot = UUtilityLibrary::PickRandomElement( BulbSpots );
	BulbMeshComponent->SetRelativeLocationAndRotation(
		PickedBulbSpot->GetRelativeLocation(),
		PickedBulbSpot->GetRelativeRotation()
	);
}

void AZeroEnemy::RetrieveReferences()
{
	verifyf( IsValid( Data ), TEXT( "%s doesn't reference a DataAsset" ), *GetName() );
	verifyf( IsValid( Data->RushSpeedCurve ), TEXT( "The DataAsset must set a value for 'RushSpeedCurve'" ) );

	// Retrieve maximum rush time from the curve
	[[maybe_unused]] float Temp = 0.0f;
	Data->RushSpeedCurve->GetTimeRange( Temp, MaxRushTime );

	auto BodyParts = UUtilityLibrary::GetComponentsOfActorByTag<UStaticMeshComponent>( 
		this,
		Data->BodyPartTag
	);
	StartBodyPartsCount = BodyParts.Num();
	LeftBodyPartsCount = StartBodyPartsCount;

	// Enable aim assist for all body parts
	for ( auto BodyPart : BodyParts )
	{
		BodyPart->SetCollisionObjectType( Data->AimAssistCollisionChannel );
	}
}

void AZeroEnemy::UpdateWalkSpeed()
{
	float WalkSpeed = Data->WalkSpeed * Modifiers.WalkSpeedMultiplier;

	// Substract speed for each body parts lost
	WalkSpeed -= Data->WalkSpeedLossPerBodyPartLost * ( StartBodyPartsCount - LeftBodyPartsCount );

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
