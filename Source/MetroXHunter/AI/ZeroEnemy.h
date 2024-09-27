#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/ZeroEnemyData.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "Health/HealthComponent.h"
#include "ZeroEnemy.generated.h"

class UAISubstateManagerComponent;

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
};

USTRUCT( BlueprintType )
struct METROXHUNTER_API FZeroEnemyModifiers
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "" )
	float WalkSpeedMultiplier = 1.0f;
};

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
	void DestroyBodyPart( USceneComponent* BodyPart );

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
	bool IsBulbOpened() const;
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	bool IsRushing() const;

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ApplyModifiers( const FZeroEnemyModifiers& NewModifiers );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ResetModifiers();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void SetState( EZeroEnemyState NewState );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	EZeroEnemyState GetState() const;

	/*
	 * Returns the madness level, representing the current progress of the AISubstateManagerComponent.
	 * In range of 0.0f to 1.0f.
	 */
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	float GetMadnessLevel() const;

	bool TakeDamage_Implementation( const FDamageContext& DamageContext ) override;

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot( struct FVisualLogEntry* Snapshot ) const override;
#endif

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnStun );
	UPROPERTY( BlueprintAssignable, EditDefaultsOnly, Category = "ZeroEnemy" )
	FOnStun OnStun;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUnStun );
	UPROPERTY( BlueprintAssignable, EditDefaultsOnly, Category = "ZeroEnemy" )
	FOnStun OnUnStun;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnRush );
	UPROPERTY( BlueprintAssignable, EditDefaultsOnly, Category = "ZeroEnemy" )
	FOnRush OnRush;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUnRush );
	UPROPERTY( BlueprintAssignable, EditDefaultsOnly, Category = "ZeroEnemy" )
	FOnUnRush OnUnRush;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnMeleeAttack, bool, bIsStarting );
	UPROPERTY( BlueprintAssignable, EditDefaultsOnly, Category = "ZeroEnemy" )
	FOnMeleeAttack OnMeleeAttack;

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UStaticMeshComponent* ProtoMeshComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UStaticMeshComponent* BulbMeshComponent = nullptr;
	
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UHealthComponent* HealthComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ZeroEnemy" )
	UAISubstateManagerComponent* AISubstateManagerComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "ZeroEnemy" )
	UZeroEnemyData* Data = nullptr;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "ZeroEnemy" )
	TSubclassOf<UAISubstate> SpawnSubstateClass;

	UPROPERTY( BlueprintReadOnly, Category = "ZeroEnemy" )
	int32 LeftBodyPartsCount = 0;

	// TODO: Move to private
	UPROPERTY( BlueprintReadOnly, Category = "ZeroEnemy" )
	bool bIsAlive = true;

private:
	void InitializeAISubstateManager();
	void GenerateBulb();
	void RetrieveReferences();

	void UpdateWalkSpeed();

private:
	bool bIsBulbOpened = false;

	EZeroEnemyState State = EZeroEnemyState::None;

	// TODO: Refactor with states
	bool bUseStunAnimation = true;

	FRotator StartStunRotation {};

	int32 StartBodyPartsCount = 0;
	float MaxRushTime = 0.0f;

	FZeroEnemyModifiers Modifiers {};

	FTimerHandle OpeningBulbTimerHandle {};
	FTimerHandle StunTimerHandle {};
	FTimerHandle RushTimerHandle {};
};
