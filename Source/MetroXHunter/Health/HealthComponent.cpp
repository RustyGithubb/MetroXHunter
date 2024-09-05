#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
}

bool UHealthComponent::TakeDamage( FDamageContext DamageContext )
{
	ensureMsgf( DamageContext.DamageAmount > 0, TEXT("'DamageContext.DamageAmount' must be positive") );

	if ( bIsDead || bIsInvulnerable ) return false;
	
	//  Apply damage to health
	CurrentHealth = FMath::Max( DamageContext.DamageAmount, 0 );
	OnDamage.Broadcast( DamageContext );
	OnHealthUpdate.Broadcast();

	//  Trigger death
	if ( CurrentHealth <= 0 )
	{
		bIsDead = true;
		OnDeath.Broadcast( DamageContext );
	}

	return true;
}

void UHealthComponent::Heal( int32 Amount )
{
	ensureMsgf( Amount > 0, TEXT("'Amount' must be positive") );

	//  Apply heal to health
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
