#include "AI/ZeroEnemyAIController.h"
#include "AI/ZeroEnemy.h"

#include "Navigation/CrowdFollowingComponent.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/KismetSystemLibrary.h"

#include "MXHUtilityLibrary.h"

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
	CurrentState = State;
}

void AZeroEnemyAIController::SetTarget( AActor* InTarget )
{
	Blackboard->SetValueAsObject( TARGET_KEYNAME, InTarget );
	Target = InTarget;

	UMXHUtilityLibrary::PrintMessage( 
		TEXT( "AI: '%s' targeting '%s'" ),
		*GetName(),
		InTarget == nullptr ? TEXT( "nullptr" ) : *InTarget->GetName()
	);
}

void AZeroEnemyAIController::TickDebugDraw()
{
	FFormatNamedArguments Args {};
	Args.Add( TEXT( "Name" ), FText::FromString( CustomPawn->GetName() ) );
	Args.Add( TEXT( "State" ), FText::FromString( UEnum::GetValueAsString( CurrentState ) ) );
	Args.Add(
		TEXT( "Target" ),
		IsValid( Target )
			? FText::FromString( Target->GetName() )
			: FText::FromString( TEXT( "nullptr" ) )
	);

	FText Text = FText::Format(
		FTextFormat::FromString(
			"Self: {Name}:\n"
			"State: {State}\n"
			"Target: {Target}\n"
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
	SetState( EZeroEnemyAIState::RushAttack );
}

void AZeroEnemyAIController::OnUnRush()
{
	SetState( EZeroEnemyAIState::Combat );
}
