/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Checkpoint/Saveable.h"
#include "HealthComponent.generated.h"

class UHealthComponent;

/*
 * Represents the damage type to apply.
 * Some types would have different effects on specific implementations, for example the 'Shock'
 * type could apply a visual effect on the player's HUD.
 */
UENUM( BlueprintType, meta = ( ScriptName = "EMetroDamageType" ) )
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
	 * Damage dealt by a spit from Infected
	 */
	Spit,
	/*
	 * Damage dealt by an extinctor exploding
	 */
	Extinctor,
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
	 * Component currently taking damage.
	 * Assigned automatically during a HealthComponent::TakeDamage call.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	UHealthComponent* HealthComponent = nullptr;
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
	 * Called when the HealthComponent is about to take damage
	 * and allows to control whenever the damage should be applied,
	 * as well as tweaking the damage context.
	 *
	 * This function is called even if the HealthComponent is dead
	 * or invulnerable.
	 *
	 * @param DamageContext Damage context
	 * @return Whenever the damage should be applied
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "HealthHolder" )
	bool TakeDamage( UPARAM( ref ) FDamageContext& DamageContext );

	/*
	 * Called when the HealthComponent is about to take damage
	 * and allows to control whenever the IHealthHolder::TakeDamage
	 * function should be called.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "HealthHolder" )
	bool CanCallTakeDamage( const FDamageContext& DamageContext );
	virtual bool CanCallTakeDamage_Implementation( const FDamageContext& DamageContext );

	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "HealthHolder" )
	bool IsShownAsDamageableToPlayer( EDamageType DamageType, const FHitResult& HitResult );
	virtual bool IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType, const FHitResult& HitResult );
};

/*
 * Component handling health with basic damage and heal features.
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UHealthComponent : public UActorComponent, public ISaveable
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void BeginPlay() override;

	// Begin ISaveable Interface
	virtual void OnSaveData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame );
	virtual void OnLoadData_Implementation( const FGuid& ActorID, UMetroSaveGame* SaveGame );
	// End ISaveable Interface

	/*
	 * Applies damage with given context. Resulted health is clamped to 0.
	 *
	 * @param DamageContext Damage context
	 */
	UFUNCTION( BlueprintCallable, Category = "Health", meta = ( ReturnDisplayName = "bTookDamage" ) )
	bool TakeDamage( FDamageContext DamageContext );
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

	/*
	 * Returns whenever the component is considered alive or dead.
	 * It is considered dead after taking damage responsible for putting the health
	 * below or equal to zero.
	 */
	UFUNCTION( BlueprintPure, Category = "Health" )
	bool IsAlive() const;

	/*
	 * Returns a ratio of the current health.
	 */
	UFUNCTION( BlueprintPure, Category = "Health" )
	float GetHealthRatio() const;

	UFUNCTION( BlueprintPure, Category = "Health" )
	bool HasHealthHolderInterface() const;

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

	/*
	 * Set whenever blood particles should be emitted when the component takes damage.
	 * This is specifically used for the Gun.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Health" )
	bool bShouldEmitBloodParticles = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Health" )
	bool bIsInvulnerable = false;

private:
	UPROPERTY( VisibleAnywhere, Category = "Health" )
	bool bIsDead = false;

	/*
	 * Does the owner implement the IHealthHolder interface?
	 */
	bool bHasHealthHolder = false;
};