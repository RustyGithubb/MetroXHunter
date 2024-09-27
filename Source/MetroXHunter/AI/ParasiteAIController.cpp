/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/ParasiteAIController.h"
#include "AI/Parasite.h"

#include "Navigation/CrowdFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

 // Set default FollowingComponent to CrowdFollowingComponent so they move around each other
AParasiteAIController::AParasiteAIController( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>( TEXT( "PathFollowingComponent" ) ) )
{
}

void AParasiteAIController::OnPossess( APawn* InPawn )
{
	CustomPawn = CastChecked<AParasite>( InPawn );

	verifyf( RunBehaviorTree( BehaviorTree ), TEXT( "Behavior Tree of %s failed to run" ), *GetName() );

	// Disabling crowd simulation fixes a bug where AI can't move on navmesh; but it disables crowd 
	// simulation features so it's just a quick patch before finding the real source
	if ( /*CustomPawn->Data->bIsCrowdSimulationDisabled*/ true )
	{
		GetComponentByClass<UCrowdFollowingComponent>()->SetCrowdSimulationState( ECrowdSimulationState::Disabled );
	}

	Super::OnPossess( InPawn );
}

#if ENABLE_VISUAL_LOG
void AParasiteAIController::GrabDebugSnapshot( FVisualLogEntry* Snapshot ) const
{
	Super::GrabDebugSnapshot( Snapshot );

	FVisualLogStatusCategory Category( TEXT( "ParasiteAI" ) );
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