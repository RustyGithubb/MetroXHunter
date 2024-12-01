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

bool UHealthComponent::TakeDamage( FDamageContext DamageContext )
{
	verify( DamageContext.DamageAmount > 0 );

	// Assign extra informations
	DamageContext.HealthComponent = this;

	// Check for owner interface
	AActor* Owner = GetOwner();
	if ( bHasHealthHolder && IHealthHolder::Execute_CanCallTakeDamage( Owner, DamageContext ) )
	{
		if ( !IHealthHolder::Execute_TakeDamage( Owner, DamageContext ) ) return false;
	}

	if ( bIsDead || bIsInvulnerable ) return false;

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

bool UHealthComponent::IsAlive() const
{
	return !bIsDead;
}
