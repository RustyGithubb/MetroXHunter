#include "AI/ZeroEnemy.h"

#include "MXHUtilityLibrary.h"

#include "Kismet/KismetSystemLibrary.h"

#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AZeroEnemy::AZeroEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	ProtoMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "ProtoMeshComponent" ) );
	ProtoMeshComponent->SetupAttachment( RootComponent );

	BulbMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "BulbMeshComponent" ) );
	BulbMeshComponent->SetupAttachment( RootComponent );
}

void AZeroEnemy::BeginPlay()
{
	Super::BeginPlay();

	RetrieveReferences();
	
	UpdateWalkSpeed();

	GenerateBulb();
	CloseBulb();
}
	
void AZeroEnemy::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if ( bIsStun )
	{
		double AngleOffset = FMath::Sin( 
			GetGameTimeSinceCreation() * Data->StunAnimationFrequency 
		) * Data->StunAnimationAngle;

		SetActorRotation( 
			FRotator {
				0.0,
				StartStunRotation.Yaw + AngleOffset,
				0.0
			}
		);

		return;
	}

	if ( bIsRushing )
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		float CurrentRushTime = TimerManager.GetTimerElapsed( RushTimerHandle );

		GetCharacterMovement()->MaxWalkSpeed = Data->RushSpeedCurve->GetFloatValue( CurrentRushTime );
		AddMovementInput( GetActorForwardVector() );
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
}

void AZeroEnemy::CloseBulb()
{
	BulbMeshComponent->SetMaterial( 0, Data->ClosedBulbMaterial );

	// Disable aim assist on bulb
	BulbMeshComponent->SetCollisionObjectType( Data->DefaultBodyPartCollisionChannel );

	OpeningBulbTimerHandle.Invalidate();
	bIsBulbOpened = false;
}

void AZeroEnemy::Stun( float StunTime )
{
	bIsStun = true;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(
		StunTimerHandle,
		this, &AZeroEnemy::UnStun,
		StunTime
	);

	StartStunRotation = GetActorRotation();

	OnStun.Broadcast();
}

void AZeroEnemy::UnStun()
{
	StunTimerHandle.Invalidate();
	bIsStun = false;

	OnUnStun.Broadcast();
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
	bIsRushing = true;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(
		RushTimerHandle,
		this, &AZeroEnemy::StopRushAttack,
		//Data->RushTime
		MaxRushTime
	);

	//GetCharacterMovement()->MaxWalkSpeed = Data->RushSpeed;

	OnRush.Broadcast();
}

void AZeroEnemy::StopRushAttack()
{
	RushTimerHandle.Invalidate();
	bIsRushing = false;

	// Reset walk speed
	UpdateWalkSpeed();

	OnUnRush.Broadcast();
}

void AZeroEnemy::GenerateBulb()
{
	auto BulbSpots = UMXHUtilityLibrary::GetComponentsOfActorByTag<UArrowComponent>(
		this,
		Data->BulbSpotTag
	);

	auto PickedBulbSpot = UMXHUtilityLibrary::PickRandomElement( BulbSpots );
	BulbMeshComponent->SetRelativeLocationAndRotation(
		PickedBulbSpot->GetRelativeLocation(),
		PickedBulbSpot->GetRelativeRotation()
	);
}

void AZeroEnemy::RetrieveReferences()
{
	verifyf( IsValid( Data ), TEXT( "%s doesn't reference a DataAsset" ), *GetName() );
	verifyf( IsValid( Data->RushSpeedCurve ), TEXT( "The DataAsset must set a value for RushSpeedCurve" ) );

	[[maybe_unused]] float Temp = 0.0f;
	Data->RushSpeedCurve->GetTimeRange( Temp, MaxRushTime );

	auto BodyParts = UMXHUtilityLibrary::GetComponentsOfActorByTag<UStaticMeshComponent>( 
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
	float WalkSpeed = Data->WalkSpeed;

	// Substract speed for each body parts lost
	WalkSpeed -= Data->WalkSpeedLossPerBodyPartLost * ( StartBodyPartsCount - LeftBodyPartsCount );

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}
