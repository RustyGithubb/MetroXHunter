#include "AI/ZeroEnemyAIController.h"
#include "AI/ZeroEnemy.h"

#include "MXHUtilityLibrary.h"

#include "Navigation/CrowdFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/KismetSystemLibrary.h"

constexpr auto STATE_KEYNAME = TEXT( "CurrentState" );
constexpr auto TARGET_KEYNAME = TEXT( "TargetActor" );

// Set default FollowingComponent to CrowdFollowingComponent so they move around each other
AZeroEnemyAIController::AZeroEnemyAIController( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>( TEXT( "PathFollowingComponent" ) ) )
{}

void AZeroEnemyAIController::OnPossess( APawn* InPawn )
{
	CustomPawn = CastChecked<AZeroEnemy>( InPawn );
	CustomPawn->OnStun.AddDynamic( this, &AZeroEnemyAIController::OnStun );
	CustomPawn->OnUnStun.AddDynamic( this, &AZeroEnemyAIController::OnUnStun );
	CustomPawn->OnRush.AddDynamic( this, &AZeroEnemyAIController::OnRush );
	CustomPawn->OnUnRush.AddDynamic( this, &AZeroEnemyAIController::OnUnRush );

	verifyf( RunBehaviorTree( BehaviorTree ), TEXT( "Behavior Tree of %s failed to run" ), *GetName() );

	// Disabling crowd simulation fixes a bug where AI can't move on navmesh; but it disables crowd 
	// simulation features so it's just a quick patch before finding the real source
	if ( CustomPawn->Data->bIsCrowdSimulationDisabled )
	{
		GetComponentByClass<UCrowdFollowingComponent>()->SetCrowdSimulationState( ECrowdSimulationState::Disabled );
	}

	Super::OnPossess( InPawn );
}

void AZeroEnemyAIController::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if ( UMXHUtilityLibrary::IsCVarAIDebugEnabled() )
	{
		TickDebugDraw();
	}
}

void AZeroEnemyAIController::CombatTarget( AActor* InTarget )
{
	SetTarget( InTarget );
	SetState( EZeroEnemyAIState::Combat );
}

void AZeroEnemyAIController::SetState( EZeroEnemyAIState State )
{
	Blackboard->SetValueAsEnum( STATE_KEYNAME, (uint8)State );
}

EZeroEnemyAIState AZeroEnemyAIController::GetState() const
{
	return (EZeroEnemyAIState)Blackboard->GetValueAsEnum( STATE_KEYNAME );
}

void AZeroEnemyAIController::SetTarget( AActor* InTarget )
{
	Blackboard->SetValueAsObject( TARGET_KEYNAME, InTarget );

	UMXHUtilityLibrary::PrintMessage(
		TEXT( "AI: '%s' targeting '%s'" ),
		*GetName(),
		*GetNameSafe( InTarget )
	);
}

AActor* AZeroEnemyAIController::GetTarget() const
{
	auto Target = Blackboard->GetValueAsObject( TARGET_KEYNAME );
	if ( !Target ) return nullptr;

	return CastChecked<AActor>( Target );
}

#if ENABLE_VISUAL_LOG
void AZeroEnemyAIController::GrabDebugSnapshot( FVisualLogEntry* Snapshot ) const
{
	Super::GrabDebugSnapshot( Snapshot );

	FVisualLogStatusCategory Category;
	Category.Category = TEXT( "Zero Enemy AI" );
	Category.Add(
		TEXT( "VelocityLength" ),
		FString::SanitizeFloat( CustomPawn->GetCharacterMovement()->Velocity.Length() )
	);
	Category.Add(
		TEXT( "MaxWalkSpeed" ),
		FString::SanitizeFloat( CustomPawn->GetCharacterMovement()->MaxWalkSpeed )
	);

	Snapshot->Status.Add( Category );
}
#endif

void AZeroEnemyAIController::TickDebugDraw()
{
	AActor* Target = GetTarget();

	FNumberFormattingOptions NumberFormattingOptions {};
	NumberFormattingOptions.MaximumFractionalDigits = 0;

	FFormatNamedArguments Args {};
	Args.Add( TEXT( "Name" ), FText::FromString( GetName() ) );
	Args.Add( TEXT( "State" ), FText::FromString( UEnum::GetValueAsString( GetState() ) ) );
	Args.Add(
		TEXT( "Target" ),
		IsValid( Target )
			? FText::FromString( Target->GetName() )
			: FText::FromString( TEXT( "nullptr" ) )
	);
	Args.Add( TEXT( "MaxWalkSpeed" ), CustomPawn->GetCharacterMovement()->MaxWalkSpeed );
	Args.Add( TEXT( "VelocityLength" ),
		FText::AsNumber(
			CustomPawn->GetCharacterMovement()->Velocity.Length(),
			&NumberFormattingOptions
		)
	);

	float DistanceFromTarget = 0.0f;
	if ( IsValid( Target ) )
	{
		DistanceFromTarget = FVector::Distance( Target->GetActorLocation(), CustomPawn->GetActorLocation() );
	}
	Args.Add( TEXT( "DistanceFromTarget" ), FText::AsNumber( DistanceFromTarget, &NumberFormattingOptions ) );

	const FText Text = FText::Format(
		FTextFormat::FromString(
			"Self: {Name}:\n"
			"State: {State}\n"
			"Target: {Target}\n"
			"VelocityLength: {VelocityLength} cm/s\n"
			"MaxWalkSpeed: {MaxWalkSpeed} cm/s\n"
			"DistanceFromTarget: {DistanceFromTarget} cm\n"
		),
		Args
	);

	UKismetSystemLibrary::DrawDebugString(
		this,
		FVector::ZeroVector,
		Text.ToString(),
		CustomPawn,
		CustomPawn->bIsAlive ? FLinearColor::White : FLinearColor::Gray
	);
}

void AZeroEnemyAIController::OnStun()
{
	SetState( EZeroEnemyAIState::Stun );
}

void AZeroEnemyAIController::OnUnStun()
{
	SetState( EZeroEnemyAIState::Combat );
}

void AZeroEnemyAIController::OnRush()
{
	//SetState( EZeroEnemyAIState::RushAttack );
}

void AZeroEnemyAIController::OnUnRush()
{
	//SetState( EZeroEnemyAIState::Combat );
}
