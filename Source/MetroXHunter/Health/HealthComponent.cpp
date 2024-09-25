#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bHasHealthHolder = GetOwner()->Implements<UHealthHolder>();
}

bool UHealthComponent::TakeDamage( const FDamageContext& DamageContext )
{
	verify( DamageContext.DamageAmount > 0 );

	if ( bIsDead || bIsInvulnerable ) return false;

	// Check for owner interface
	if ( bHasHealthHolder && !IHealthHolder::Execute_TakeDamage( GetOwner(), DamageContext ) ) return false;

	// Apply damage to health
	CurrentHealth = FMath::Max( CurrentHealth - DamageContext.DamageAmount, 0 );
	OnDamage.Broadcast( DamageContext );
	OnHealthUpdate.Broadcast();

	// Trigger death
	if ( CurrentHealth <= 0 )
	{
		bIsDead = true;
		OnDeath.Broadcast( DamageContext );
	}

	return true;
}

void UHealthComponent::Heal( int32 Amount )
{
	verify( Amount > 0 );

	// Apply heal to health
	CurrentHealth = FMath::Min( CurrentHealth + Amount, MaxHealth );
	bIsDead = false;

	OnHealthUpdate.Broadcast();
}

void UHealthComponent::Reset()
{
	CurrentHealth = MaxHealth;
	bIsDead = false;

	OnHealthUpdate.Broadcast();
}
