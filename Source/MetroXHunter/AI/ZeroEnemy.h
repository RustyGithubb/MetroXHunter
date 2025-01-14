/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "AI/ZeroEnemyData.h"
#include "AI/ZeroEnemyAISubstate.h"

#include "Health/HealthComponent.h"

#include "GameFramework/Character.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"

#include "ZeroEnemy.generated.h"

class UPawnSensingComponent;
class UElectrocutableComponent;

UENUM( BlueprintType )
enum class EZeroEnemyState : uint8
{
	/*
	 * No specific restrictions.
	 */
	None,
	/*
	 * Unable to move for a given time, also used for animating.
	 */
	Stun,
	/*
	 * Same as Stun, specific for a different animation.
	 */
	//SubstateChange,
	/*
	 * Attacking a target by charging it.
	 */
	RushAttack,
	/*
	 * Waiting the rush to a player to be resolved by the quick time event.
	 */
	RushAttackResolve,
	/*
	 * Attacking a target with melee.
	 */
	MeleeAttack,
	/*
	 * As name implies, currently faking death.
	 */
	FakingDeath,
	/*
	 * Currently knocked out and ragdolled for a given time.
	 */
	KnockOut,
};

USTRUCT( BlueprintType )
struct METROXHUNTER_API FZeroEnemyModifiers
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "" )
	float WalkSpeedMultiplier = 1.0f;
};

constexpr int32 MAX_NUM_ZERO_ENEMY_LEGS = 4;

UCLASS( Abstract )
class METROXHUNTER_API AZeroEnemy :
	public ACharacter, public IHealthHolder, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

public:
	AZeroEnemy();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void FakeDeath();
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void UnFakeDeath();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void KnockOut();
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void UnKnockOut();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void Ragdoll();
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void UnRagdoll();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SimulateMeshBonesPhysics( bool bSimulate );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SetCollisionsEnabled( bool bEnabled );

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void OpenBulb( float OpenTime = 0.0f );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void CloseBulb();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void Stun( float StunTime = 0.0f, bool bUseDefaultAnimation = true );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void UnStun();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void MakePanic();
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	bool DestroyBodyPart(
		USceneComponent* BodyPart,
		const FName& BoneName,
		const FVector& HitLocation,
		const FVector& KnockbackDirection,
		const float DistanceFromAttacker
	);
	int32 GetStartingBodyPartsCount() const;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ApplyKnockback( const FVector& Direction, float Force );

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void RushAttack();
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void StartResolveRushAttack();
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void StopRushAttack();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "ZeroEnemy" )
	void MeleeAttack();
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "ZeroEnemy" )
	void StopMeleeAttack();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SpitAttack( const FVector& TargetLocation );
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable, Category = "ZeroEnemy" )
	FVector GetSpitAttackOrigin() const;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	bool IsBulbOpened() const;
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	bool IsRushing() const;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void Scream();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ApplyModifiers( const FZeroEnemyModifiers& NewModifiers );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ResetModifiers();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SetState( EZeroEnemyState NewState );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	EZeroEnemyState GetState() const;

	bool CanCallTakeDamage_Implementation( const FDamageContext& DamageContext ) override;
	bool TakeDamage_Implementation( FDamageContext& DamageContext ) override;

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot( struct FVisualLogEntry* Snapshot ) const override;
#endif

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnStun );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnStun OnStun {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUnStun );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnStun OnUnStun {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnRush );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnRush OnRush {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUnRush );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnUnRush OnUnRush {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnMeleeAttack, bool, bIsStarting );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnMeleeAttack OnMeleeAttack {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnStateUpdate, EZeroEnemyState, NewState, EZeroEnemyState, OldState );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnStateUpdate OnStateUpdate {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnAttacked, AActor*, Attacker );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnAttacked OnAttacked {};

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UStaticMeshComponent* BulbMeshComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UElectrocutableComponent* ElectrocutableComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UPawnSensingComponent* PawnSensingComponent = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "ZeroEnemy" )
	USaveLoadComponent* SaveComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ZeroEnemy", meta = ( ExposeOnSpawn = true ) )
	UZeroEnemyData* Data = nullptr;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "ZeroEnemy" )
	TSubclassOf<UZeroEnemyAISubstate> SpawnSubstateClass;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	TArray<FName> SimulatedMeshBones {};

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	TArray<FName> SimulatedMeshBonesBelow {};

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "ZeroEnemy" )
	bool bStartFakingDeath = false;

	UPROPERTY( BlueprintReadOnly, Category = "ZeroEnemy" )
	int32 LeftBodyPartsCount = MAX_NUM_ZERO_ENEMY_LEGS;

private:
	void GenerateBulb();
	void RetrieveReferences();

	void UpdateWalkSpeed();

	UFUNCTION()
	void OnElectricStart( float Duration );

	UFUNCTION()
	void OnDeath( const FDamageContext& DamageContext );

	UFUNCTION()
	void OnRagdollMeshHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	void SpawnBloodPuddle();

private:
	bool bIsBulbOpened = false;

	EZeroEnemyState State = EZeroEnemyState::None;

	// TODO: Refactor with states
	bool bUseStunAnimation = true;

	FTransform DefaultMeshRelativeTransform {};
	FCollisionResponseContainer DefaultMeshCollisions {};
	FRotator StartStunRotation {};

	int32 StartBodyPartsCount = 0;
	float MaxRushTime = 0.0f;
	float MaxKnockOutDistance = -1.0f;

	/*
	 * Specify which leg is dead or not.
	 * Indexes: Front Left = 0, Front Right = 1, Back Left = 2, Back Right = 3
	 */
	bool DeadLegs[MAX_NUM_ZERO_ENEMY_LEGS] {};

	FZeroEnemyModifiers Modifiers {};

	FTimerHandle OpeningBulbTimerHandle {};
	FTimerHandle StunTimerHandle {};
	FTimerHandle RushTimerHandle {};
	FTimerHandle KnockOutTimerHandle {};
};
