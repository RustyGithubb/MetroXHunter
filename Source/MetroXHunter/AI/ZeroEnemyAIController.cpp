#include "AI/ZeroEnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "MXHUtilityLibrary.h"

constexpr auto STATE_KEYNAME = TEXT( "CurrentState" );
constexpr auto TARGET_KEYNAME = TEXT( "TargetActor" );

void AZeroEnemyAIController::OnPossess( APawn* InPawn )
{
	ensure( RunBehaviorTree( BehaviorTree ) );
	UMXHUtilityLibrary::PrintMessage( TEXT( "AI: Behavior Tree for '%s' is running!" ), *GetName() );

	Super::OnPossess( InPawn );
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
