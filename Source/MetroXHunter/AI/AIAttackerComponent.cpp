/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/AIAttackerComponent.h"
#include "AI/AITargetComponent.h"

UAIAttackerComponent::UAIAttackerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAIAttackerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAIAttackerComponent::EndPlay( const EEndPlayReason::Type EndPlayReason )
{
	if ( !IsValid( CurrentTarget ) ) return;

	CurrentTarget->FreeReservations( GetOwner() );
}

bool UAIAttackerComponent::ReserveTokens( UAITargetComponent* Target, int32 Tokens )
{
	verify( IsValid( Target ) );

	if ( IsValid( CurrentTarget ) && Target != CurrentTarget )
	{
		FreeTokens();
	}

	CurrentTarget = Target;

	bool bIsSuccess = CurrentTarget->ReserveTokens( GetOwner(), Tokens );
	return bIsSuccess;
}

bool UAIAttackerComponent::FreeTokens( int32 Tokens )
{
	if ( !IsValid( CurrentTarget ) ) return false;

	bool bIsSuccess = CurrentTarget->FreeTokens( GetOwner(), Tokens );
	CurrentTarget = nullptr;

	return bIsSuccess;
}

UAITargetComponent* UAIAttackerComponent::GetCurrentTarget() const
{
	return CurrentTarget;
}
