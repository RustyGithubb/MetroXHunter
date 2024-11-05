/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/AITargetComponent.h"
#include "AI/AIAttackerComponent.h"

UAITargetComponent::UAITargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAITargetComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAITargetComponent::TickDebug_Implementation( float DeltaTime, FString& OutDebugText )
{
	// Construct reservations string 
	FString Reservations = "";
	for ( const auto& Pair : ReservedTokens )
	{
		const UAIAttackerComponent* Reserver = Pair.Key;
		Reservations += "- " + GetNameSafe( Reserver->GetReserver() ) + ": " + FString::FromInt( Pair.Value ) + "\n";
	}

	// Construct attackers string 
	FString GroupPlaces = "";
	for ( const auto& Pair : ReservedGroupPlaces )
	{
		const UAIAttackerComponent* Reserver = Pair.Key;
		GroupPlaces += "- " + GetNameSafe( Reserver->GetReserver() ) + " : " + FString::FromInt( Pair.Value ) + "\n";
	}

	// Construct attackers string 
	FString Attackers = "";
	for ( const auto& Reserver : DeclaredAttackers )
	{
		Attackers += "- " + GetNameSafe( Reserver->GetReserver() ) + "\n";
	}

	// Construct formating arguments
	FStringFormatNamedArguments Args {};
	Args.Add( "RemainingTokens", GetRemainingTokens() );
	Args.Add( "MaxTokens", MaxTokens );
	Args.Add( "TokenCooldown", GetTokenCooldown() );
	Args.Add( "Reservations", Reservations );
	Args.Add( "ReservationsCount", ReservedTokens.Num() );
	Args.Add( "GroupPlaces", GroupPlaces );
	Args.Add( "GroupPlacesCount", ReservedGroupPlaces.Num() );
	Args.Add( "Attackers", Attackers );
	Args.Add( "AttackersCount", DeclaredAttackers.Num() );

	// Format debug string
	constexpr auto Format = TEXT( 
		"[AITargetComponent]\n"
		"Tokens: {RemainingTokens}/{MaxTokens}\n"
		"TokenCooldown: {TokenCooldown}\n"
		"Reservations[{ReservationsCount}]:\n{Reservations}"
		"GroupPlaces[{GroupPlacesCount}]:\n{GroupPlaces}"
		"Attackers[{AttackersCount}]:\n{Attackers}"
	);
	OutDebugText = FString::Format( Format, Args );
}

bool UAITargetComponent::ReserveTokens( UAIAttackerComponent* Reserver, int32 Tokens )
{
	verify( Reserver != nullptr );

	// Check for cooldown
	if ( GetTokenCooldown() > 0.0f )
	{
		UE_VLOG(
			GetOwner(),
			LogTemp, Verbose,
			TEXT( "AITargetComponent: Failed to reserve %d tokens for %s: token reservation in cooldown." ),
			Tokens, *GetNameSafe( Reserver )
		);
		return false;
	}

	// Check if enough tokens are available
	if ( Tokens > GetRemainingTokens() ) 
	{
		UE_VLOG(
			GetOwner(),
			LogTemp, Verbose,
			TEXT( "AITargetComponent: Failed to reserve %d tokens for %s: not enough tokens available." ),
			Tokens, *GetNameSafe( Reserver )
		);
		return false;
	}

	// Add new tokens, in order to keep the already reserved tokens
	const int32 ReserverTokens = GetReservedTokens( Reserver );
	const int32 ResultingTokens = Tokens + ReserverTokens;

	ReservedTokens.Add( Reserver, ResultingTokens );

	UE_VLOG(
		GetOwner(),
		LogTemp, Verbose,
		TEXT( "AITargetComponent: Reserved %d tokens for %s (now a total of %d tokens)" ),
		Tokens, *GetNameSafe( Reserver ), ResultingTokens
	);

	return true;
}

bool UAITargetComponent::FreeTokens( UAIAttackerComponent* Reserver, int32 Tokens )
{
	verify( Reserver != nullptr );

	const int32 ReserverTokens = GetReservedTokens( Reserver );
	if ( ReserverTokens == 0 )
	{
		UE_VLOG(
			GetOwner(),
			LogTemp, Verbose,
			TEXT( "AITargetComponent: Failed to free %d tokens for %s: no reservations." ),
			Tokens, *GetNameSafe( Reserver )
		);
		return false;
	}
	
	// Automatically fill with reserved tokens if set to zero
	if ( Tokens == 0 )
	{
		Tokens = ReserverTokens;
	}

	// Remove the reserver if we result to zero...
	const int32 ResultingTokens = FMath::Max( 0, ReserverTokens - Tokens );
	if ( ResultingTokens == 0 )
	{
		ReservedTokens.Remove( Reserver );
	}
	// ...or set to resulting tokens
	else
	{
		ReservedTokens.Add( Reserver, ResultingTokens );
	}

	UE_VLOG(
		GetOwner(),
		LogTemp, Verbose,
		TEXT( "AITargetComponent: Freed %d tokens for %s (now a total of %d tokens)" ),
		Tokens, *GetNameSafe( Reserver ), ResultingTokens
	);

	return true;
}

void UAITargetComponent::ClearTokens()
{
	ReservedTokens.Empty();
}

void UAITargetComponent::SetTokenCooldown( float Seconds )
{
	UE_VLOG(
		GetOwner(),
		LogTemp, Verbose,
		TEXT( "AITargetComponent: Set token cooldown to %.2fs" ),
		Seconds
	);

	EndTokenCooldownTime = GetWorld()->GetTimeSeconds() + Seconds;
}

int32 UAITargetComponent::GetReservedTokens( UAIAttackerComponent* Reserver ) const
{
	verify( Reserver != nullptr );

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

	return RemainingTokens;
}

float UAITargetComponent::GetTokenCooldown() const
{
	return EndTokenCooldownTime - GetWorld()->GetTimeSeconds();
}

bool UAITargetComponent::ReserveGroupPlace( UAIAttackerComponent* Reserver, int32& GroupIndex )
{
	verify( Reserver != nullptr );

	for ( int Index = 0; Index < GroupsSettings.Num(); Index++ )
	{
		const auto& GroupSettings = GroupsSettings[Index];

		// If there are infinite places, reserve this group
		if ( GroupSettings.MaxPlaces == 0 )
		{
			GroupIndex = Index;
			ReservedGroupPlaces.Add( Reserver, GroupIndex );
			Reserver->SetGroupPlace( GroupIndex );

			return true;
		}

		// If there are no remaining places, skip to the next group
		if ( GetRemainingGroupPlaces( Index ) <= 0 ) continue;
	
		GroupIndex = Index;
		ReservedGroupPlaces.Add( Reserver, GroupIndex );
		Reserver->SetGroupPlace( GroupIndex );

		return true;
	}

	GroupIndex = -1;
	return false;
}

void UAITargetComponent::MoveGroupPlace( UAIAttackerComponent* Reserver, int32 NewGroupIndex )
{
	verify( Reserver != nullptr );

	ReservedGroupPlaces.Add( Reserver, NewGroupIndex );
	Reserver->SetGroupPlace( NewGroupIndex );
}

bool UAITargetComponent::FreeGroupPlace( UAIAttackerComponent* Reserver )
{
	verify( Reserver != nullptr );

	const int32 GroupIndex = GetReservedGroupPlace( Reserver );
	if ( GroupIndex == -1 ) return false;

	ReservedGroupPlaces.Remove( Reserver );
	Reserver->SetGroupPlace( -1 );

	// Don't move actors from groups if the group has infinite places or it was the last group
	if ( GroupsSettings[GroupIndex].MaxPlaces == 0 ) return true;
	if ( GroupIndex < GroupsSettings.Num() - 1 ) return true;

	// Move the first actor from the next group to this group
	// NOTE: Probably too much for current implementation
	auto ActorsByPlaces = GetActorsByGroupPlaces();
	auto Actors = ActorsByPlaces.Find( GroupIndex + 1 );
	if ( Actors == nullptr ) return true;

	MoveGroupPlace( Actors->Data[0], GroupIndex );

	return true;
}

const FAITargetGroupSettings& UAITargetComponent::GetGroupSettings( int32 GroupIndex ) const
{
	verifyf(
		GroupsSettings.IsValidIndex( GroupIndex ),
		TEXT( "Out-of-bounds with index %d" ), GroupIndex
	);
	return GroupsSettings[GroupIndex];
}

int32 UAITargetComponent::GetReservedGroupPlace( UAIAttackerComponent* Reserver ) const
{
	verify( Reserver != nullptr );

	auto Itr = ReservedGroupPlaces.Find( Reserver );
	if ( Itr == nullptr ) return -1;

	return *Itr;
}

int32 UAITargetComponent::GetRemainingGroupPlaces( int32 GroupIndex ) const
{
	int32 RemainingPlacesCount = 0;

	for ( const auto& Pair : ReservedGroupPlaces )
	{
		if ( Pair.Value != GroupIndex ) continue;
		
		RemainingPlacesCount++;
	}

	return GroupsSettings[GroupIndex].MaxPlaces - RemainingPlacesCount;
}

TMap<int32, FAIReserverArray> UAITargetComponent::GetActorsByGroupPlaces() const
{
	TMap<int32, FAIReserverArray> ActorsByPlaces {};

	for ( const auto& Pair : ReservedGroupPlaces )
	{
		UAIAttackerComponent* Reserver = Pair.Key;
		const int32 GroupIndex = Pair.Value;

		FAIReserverArray& Reservers = ActorsByPlaces.FindOrAdd( GroupIndex, {} );
		Reservers.Data.Add( Reserver );
	}

	return ActorsByPlaces;
}

void UAITargetComponent::DeclareAttacker( UAIAttackerComponent* Attacker )
{
	DeclaredAttackers.Add( Attacker );
}

void UAITargetComponent::RetireAttacker( UAIAttackerComponent* Attacker )
{
	DeclaredAttackers.Remove( Attacker );
}

int32 UAITargetComponent::GetAttackersCount() const
{
	return DeclaredAttackers.Num();
}

void UAITargetComponent::FreeReservations( UAIAttackerComponent* Reserver )
{
	verify( Reserver != nullptr );

	FreeTokens( Reserver );
	FreeGroupPlace( Reserver );
	RetireAttacker( Reserver );
}
