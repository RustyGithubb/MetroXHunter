/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/AISubstateManagerComponent.h"
#include "AI/AISubstate.h"

UAISubstateManagerComponent::UAISubstateManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAISubstateManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	//SwitchToSubstate( CurrentIndex );
}

void UAISubstateManagerComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// TODO: Implement substates switching
}

void UAISubstateManagerComponent::CreateSubstates(
	const TArray<TSubclassOf<UAISubstate>> SubstateClasses
)
{
	verifyf( !SubstateClasses.IsEmpty(), TEXT( "'SubstateClasses' must not be empty!" ) );

	// Create all substates
	for ( auto& SubstateClass : SubstateClasses )
	{
		auto Substate = NewObject<UAISubstate>( this, SubstateClass );
		Substates.Add( Substate );
	}
}

void UAISubstateManagerComponent::SwitchToSubstate( int32 Index )
{
	verifyf( 
		!Substates.IsEmpty(), 
		TEXT( "You must create substates for the 'AISubstateManagerComponent' before using it!" )
	);

	// Alert previous substate
	if ( IsValid( CurrentSubstate ) )
	{
		CurrentSubstate->OnSubstateExited();
	}

	// Switch to new substate
	CurrentIndex = Index;
	CurrentSubstate = Substates[CurrentIndex];

	// Alert new substate
	CurrentSubstate->OnSubstateEntered();
}

