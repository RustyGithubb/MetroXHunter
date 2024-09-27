/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/AITargetComponent.h"

UAITargetComponent::UAITargetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAITargetComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UAITargetComponent::ReserveTokens( AActor* Reserver, int32 Tokens )
{
	verify( IsValid( Reserver ) );

	// Check if enough tokens are available
	if ( Tokens > GetRemainingTokens() ) return false;

	// Add new tokens, in order to keep the already reserved tokens
	int32 ReserverTokens = GetReservedTokens( Reserver );
	Tokens += ReserverTokens;

	ReservedTokens.Add( Reserver, Tokens );

	return true;
}

void UAITargetComponent::FreeTokens( AActor* Reserver, int32 Tokens )
{
	int32 ReserverTokens = GetReservedTokens( Reserver );
	
	// Automatically fill with reserved tokens if set to zero
	if ( Tokens == 0 )
	{
		Tokens = ReserverTokens;
	}

	// Remove the reserver if we result to zero...
	int32 ResultingTokens = FMath::Max( 0, ReserverTokens - Tokens );
	if ( ResultingTokens == 0 )
	{
		ReservedTokens.Remove( Reserver );
	}
	// ...or set to resulting tokens
	else
	{
		ReservedTokens.Add( Reserver, ResultingTokens );
	}
}

void UAITargetComponent::ClearTokens()
{
	ReservedTokens.Empty();
}

int32 UAITargetComponent::GetReservedTokens( AActor* Reserver ) const
{
	auto Itr = ReservedTokens.Find( Reserver );
	if ( Itr == nullptr ) return 0;

	return *Itr;
}

int32 UAITargetComponent::GetRemainingTokens() const
{
	int32 RemainingTokens = MaxTokens;

	for ( const auto& Element : ReservedTokens )
	{
		RemainingTokens -= Element.Value;
	}

	return MaxTokens;
}

bool UAITargetComponent::ReserveGroupPlace( AActor* Reserver, int32& GroupIndex )
{
	for ( int Index = 0; Index < GroupsSettings.Num(); Index++ )
	{
		const auto& GroupSettings = GroupsSettings[Index];

		// If there are infinite places, reserve this group
		if ( GroupSettings.MaxPlaces == 0 )
		{
			GroupIndex = Index;
			return true;
		}

		// If there are no remaining places, skip to the next group
		if ( GetRemainingGroupPlaces( Index ) == 0 ) continue;
	
		GroupIndex = Index;
		return true;
	}

	GroupIndex = -1;
	return false;
}

void UAITargetComponent::MoveGroupPlace( AActor* Reserver, int32 NewGroupIndex )
{
	ReservedGroupPlaces.Remove( Reserver );
	ReservedGroupPlaces.Add( Reserver, NewGroupIndex );

	// TODO: Add interface to warn the Reserver of group changing
}

bool UAITargetComponent::FreeGroupPlace( AActor* Reserver )
{
	int32 GroupIndex = GetReservedGroupPlace( Reserver );
	if ( GroupIndex == -1 ) return false;

	ReservedGroupPlaces.Remove( Reserver );

	// Don't move actors from groups if the group has infinite places or it was the last group
	if ( GroupsSettings[GroupIndex].MaxPlaces == 0 ) return true;
	if ( GroupIndex < GroupsSettings.Num() - 1 ) return true;

	// Move the first actor from the next group to this group
	// NOTE: Probably too much for current implementation
	auto ActorsByPlaces = GetActorsByGroupPlaces();
	auto Actors = ActorsByPlaces.Find( GroupIndex + 1 );
	if ( Actors == nullptr ) return true;

	MoveGroupPlace( Actors[0], GroupIndex );

	return true;
}

int32 UAITargetComponent::GetReservedGroupPlace( AActor* Reserver ) const
{
	auto Itr = ReservedGroupPlaces.Find( Reserver );
	if ( Itr == nullptr ) return -1;

	return *Itr;
}

int32 UAITargetComponent::GetRemainingGroupPlaces( int32 GroupIndex ) const
{
	return int32();
}

TMap<int32, AActor*> UAITargetComponent::GetActorsByGroupPlaces() const
{
	return TMap<int32, AActor*>();
}

void UAITargetComponent::FreeReservations( AActor* Reserver )
{
	FreeTokens( Reserver );
	FreeGroupPlace( Reserver );
}
