#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/ZeroEnemyData.h"
#include "ZeroEnemy.generated.h"

class UHealthComponent;
class UAISubstateManagerComponent;

USTRUCT( BlueprintType )
struct METROXHUNTER_API FZeroEnemyModifiers
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "" )
	float WalkSpeedMultiplier = 1.0f;
};

UCLASS()
class METROXHUNTER_API AZeroEnemy : public ACharacter
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
	void Stun( float StunTime = 0.0f );
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
	void StopRushAttack();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "ZeroEnemy" )
	void MeleeAttack();
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "ZeroEnemy" )
	void StopMeleeAttack();

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	bool IsBulbOpened() const { return bIsBulbOpened; }
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	bool IsRushing() const { return bIsRushing; }

	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ApplyModifiers( const FZeroEnemyModifiers& NewModifiers );
	UFUNCTION( BlueprintCallable, Category = "ZeroEnemy" )
	void ResetModifiers();

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

	// TODO: Refactor with states
	bool bIsStun = false;
	bool bIsRushing = false;
	bool bIsMeleeAttacking = false;

	FRotator StartStunRotation {};

	int32 StartBodyPartsCount = 0;
	float MaxRushTime = 0.0f;

	FZeroEnemyModifiers Modifiers {};

	FTimerHandle OpeningBulbTimerHandle {};
	FTimerHandle StunTimerHandle {};
	FTimerHandle RushTimerHandle {};
};
