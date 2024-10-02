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

void UAITargetComponent::TickComponent( 
	float DeltaTime,
	ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction
)
{
	// Construct reservations string 
	FString Reservations = "";
	for ( const auto& Pair : ReservedTokens )
	{
		Reservations += "- " + GetNameSafe( Pair.Key ) + ": " + FString::FromInt( Pair.Value ) + "\n";
	}

	// Construct formating arguments
	FStringFormatNamedArguments Args {};
	Args.Add( "RemainingTokens", GetRemainingTokens() );
	Args.Add( "MaxTokens", MaxTokens );
	Args.Add( "TokenCooldown", GetTokenCooldown() );
	Args.Add( "Reservations", Reservations );

	// Format debug string
	constexpr auto Format = TEXT( 
		"[AITargetComponent]\n"
		"Tokens: {RemainingTokens}/{MaxTokens}\n"
		"TokenCooldown: {TokenCooldown}\n"
		"Reservations:\n"
		"{Reservations}\n"
	);
	FString DebugString = FString::Format( Format, Args );

	DrawDebugString(
		GetWorld(),
		FVector { 0.0f, 0.0f, 50.0f },
		DebugString,
		GetOwner(),
		FColor::White,
		/* Duration */ 0.0f
	);
}

bool UAITargetComponent::ReserveTokens( AActor* Reserver, int32 Tokens )
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
	int32 ReserverTokens = GetReservedTokens( Reserver );
	int32 ResultingTokens = Tokens + ReserverTokens;

	ReservedTokens.Add( Reserver, ResultingTokens );

	UE_VLOG(
		GetOwner(),
		LogTemp, Verbose,
		TEXT( "AITargetComponent: Reserved %d tokens for %s (now a total of %d tokens)" ),
		Tokens, *GetNameSafe( Reserver ), ResultingTokens
	);

	return true;
}

bool UAITargetComponent::FreeTokens( AActor* Reserver, int32 Tokens )
{
	verify( Reserver != nullptr );

	int32 ReserverTokens = GetReservedTokens( Reserver );
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

int32 UAITargetComponent::GetReservedTokens( AActor* Reserver ) const
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

bool UAITargetComponent::ReserveGroupPlace( AActor* Reserver, int32& GroupIndex )
{
	verify( Reserver != nullptr );

	// TODO: Implement actual reservation

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
		if ( GetRemainingGroupPlaces( Index ) <= 0 ) continue;
	
		GroupIndex = Index;
		return true;
	}

	GroupIndex = -1;
	return false;
}

void UAITargetComponent::MoveGroupPlace( AActor* Reserver, int32 NewGroupIndex )
{
	verify( Reserver != nullptr );

	ReservedGroupPlaces.Remove( Reserver );
	ReservedGroupPlaces.Add( Reserver, NewGroupIndex );

	// TODO: Add interface to warn the Reserver of group changing
}

bool UAITargetComponent::FreeGroupPlace( AActor* Reserver )
{
	verify( Reserver != nullptr );

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

	MoveGroupPlace( Actors->Array[0], GroupIndex );

	return true;
}

int32 UAITargetComponent::GetReservedGroupPlace( AActor* Reserver ) const
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

TMap<int32, FActorArray> UAITargetComponent::GetActorsByGroupPlaces() const
{
	TMap<int32, FActorArray> ActorsByPlaces {};

	for ( const auto& Pair : ReservedGroupPlaces )
	{
		AActor* Actor = Pair.Key;
		int32 GroupIndex = Pair.Value;

		FActorArray& GroupActors = ActorsByPlaces.FindOrAdd( GroupIndex, {} );
		GroupActors.Array.Add( Actor );
	}

	return ActorsByPlaces;
}

void UAITargetComponent::FreeReservations( AActor* Reserver )
{
	verify( Reserver != nullptr );

	FreeTokens( Reserver );
	FreeGroupPlace( Reserver );
}