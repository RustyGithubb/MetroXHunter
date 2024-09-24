#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

/*
 * Represents the damage type to apply.
 * Some types would have different effects on specific implementations, for example the 'Shock'
 * type could apply a visual effect on the player's HUD.
 */
UENUM( BlueprintType )
enum class EDamageType : uint8
{
	/*
	 * Common damage type, shoudn't do anything special
	 */
	Generic,
	/*
	 * Damage dealt by a firearm weapon
	 */
	Bullet,
	/*
	 * Damage dealt by a melee attack
	 */
	Melee,
	/*
	 * Damage dealt by electrocution
	 */
	Shock,
	/*
	 * Damage dealt by an explosion
	 */
	Explosion,
};

/*
 * Holds all the information of a damage event.
 * When calling 'TakeDamage', these fields should be filled as much as you can.
 */
USTRUCT( BlueprintType )
struct FDamageContext
{
	GENERATED_BODY()

	/*
	 * Amount of damage applied
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int32 DamageAmount = 0;
	/*
	 * Type of damage dealt
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	EDamageType DamageType = EDamageType::Generic;
	/*
	 * Actor indirectly causing the damage (e.g. the player, a spider), this could not be set.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	AActor* AttackerActor = nullptr;
	/*
	 * Actor directly causing the damage (e.g. the gun), this could not be set.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	AActor* InflictorActor = nullptr;
	/*
	 * Trace result performed by the attacker, this could not be filled
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FHitResult HitResult {};
};

UINTERFACE( Blueprintable )
class UHealthHolder : public UInterface
{
	GENERATED_BODY()
};

/*
 * Interface designed to add control over the behavior of the UHealthComponent of an actor.
 * This should be implemented on the owner.
 */
class METROXHUNTER_API IHealthHolder
{
	GENERATED_BODY()

public:
	/*
	 * Function called when the HealthComponent is taking damage 
	 * and controlling whenever the damage should be applied.
	 * 
	 * @param DamageContext Damage context
	 * @return Whenever the damage should be applied
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "HealthHolder" )
	bool TakeDamage( const FDamageContext& DamageContext );
};

/*
 * Component handling health with basic damage and heal features.
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void BeginPlay() override;

	/*
	 * Applies damage with given context. Resulted health is clamped to 0.
	 *
	 * @param DamageContext Damage context
	 */
	UFUNCTION( BlueprintCallable, Category = "Health" )
	bool TakeDamage( const FDamageContext& DamageContext );
	/*
	 * Heal by a given amount. Resulted health is clamped to 'MaxHealth'.
	 *
	 * @param Amount Amount to heal, MUST be positive (> 0)
	 */
	UFUNCTION( BlueprintCallable, Category = "Health" )
	void Heal( int32 Amount );
	/*
	 * Updates current health to maximum health.
	 */
	UFUNCTION( BlueprintCallable, Category = "Health" )
	void Reset();

public:
	/*
	 * Event called when a damage has been taken.
	 *
	 * @param DamageContext Damage context
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnDamage, const FDamageContext&, DamageContext );
	UPROPERTY( BlueprintAssignable, Category = "Health" )
	FOnDamage OnDamage;

	/*
	 * Event called when health has been changed. Useful for visual updates such as UI.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnHealthUpdate );
	UPROPERTY( BlueprintAssignable, Category = "Health" )
	FOnHealthUpdate OnHealthUpdate;

	/*
	 * Event called when death happens.
	 *
	 * @param DamageContext Damage context
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnDeath, const FDamageContext&, DamageContext );
	UPROPERTY( BlueprintAssignable, Category = "Health" )
	FOnDeath OnDeath;

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Health" )
	int32 CurrentHealth = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Health" )
	int32 MaxHealth = 0;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Health" )
	bool bIsDead = false;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Health" )
	bool bIsInvulnerable = false;

	/*
	 * Does the owner implement the IHealthHolder interface?
	 */
	bool bHasHealthHolder = false;
};
