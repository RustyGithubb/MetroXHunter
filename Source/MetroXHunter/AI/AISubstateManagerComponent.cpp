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

	if ( CurrentSubstate != nullptr )
	{
		auto SwitchOrder = CurrentSubstate->TickSubstate( DeltaTime );
		switch ( SwitchOrder )
		{
			case EAISubstateSwitchOrder::Previous:
			{
				SwitchToSubstate( CurrentIndex - 1 );
				break;
			}
			case EAISubstateSwitchOrder::Next:
			{
				SwitchToSubstate( CurrentIndex + 1 );
				break;
			}
		}
	}
}

void UAISubstateManagerComponent::CreateSubstates(
	const TArray<TSubclassOf<UAISubstate>>& SubstateClasses
)
{
	verifyf( !SubstateClasses.IsEmpty(), TEXT( "'SubstateClasses' must not be empty!" ) );

	// Create all substates
	for ( auto& SubstateClass : SubstateClasses )
	{
		auto Substate = NewObject<UAISubstate>( this, SubstateClass );
		Substate->Manager = this;
		Substate->OnSubstateInitialized();
		Substates.Add( Substate );
	}
}

bool UAISubstateManagerComponent::SwitchToSubstate( int32 Index )
{
	verifyf( 
		!Substates.IsEmpty(), 
		TEXT( "You must create substates for the 'AISubstateManagerComponent' before using it!" )
	);

	// Abort switching to an invalid index
	if ( !Substates.IsValidIndex( Index ) ) return false;

	// Alert previous substate
	bool bHasPreviousSubstate = false;
	if ( CurrentSubstate != nullptr )
	{
		CurrentSubstate->OnSubstateExited();
		bHasPreviousSubstate = true;
	}

	// Switch to new substate
	CurrentIndex = Index;
	CurrentSubstate = Substates[CurrentIndex];

	GameTimeSinceLastSwitch = GetOwner()->GetGameTimeSinceCreation();

	// Alert new substate
	CurrentSubstate->OnSubstateEntered();
	
	if ( bHasPreviousSubstate )
	{
		OnSubstateSwitched.Broadcast();
	}
	return true;
}

float UAISubstateManagerComponent::GetSubstateTime() const
{
	return GetOwner()->GetGameTimeSinceCreation() - GameTimeSinceLastSwitch;
}

