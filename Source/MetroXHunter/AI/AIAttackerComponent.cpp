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
	FreeReservations();
}

void UAIAttackerComponent::SetCurrentTarget( UAITargetComponent* Target )
{
	if ( Target == CurrentTarget ) return;

	if ( IsValid( CurrentTarget ) )
	{
		FreeReservations();
	}

	CurrentTarget = Target;
}

void UAIAttackerComponent::FreeReservations()
{
	if ( !IsValid( CurrentTarget ) ) return;

	CurrentTarget->FreeReservations( this );
}

void UAIAttackerComponent::SetGroupPlace( const int32 NewGroupIndex )
{
	const int32 LastGroupIndex = GroupIndex;
	GroupIndex = NewGroupIndex;
	OnGroupPlaceChanged.Broadcast( LastGroupIndex, NewGroupIndex );
}

int32 UAIAttackerComponent::GetGroupPlace() const
{
	return GroupIndex;
}

AActor* UAIAttackerComponent::GetReserver() const
{
	return GetOwner();
}

UAITargetComponent* UAIAttackerComponent::GetCurrentTarget() const
{
	return CurrentTarget;
}
