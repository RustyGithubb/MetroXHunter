/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Health/HealthComponent.h"
#include "AI/ParasiteData.h"
#include "AI/EQSContextProvider.h"
#include "Parasite.generated.h"

class AZeroEnemy;
class APossessableCorpse;
class AVent;

class UHealthComponent;
class UPawnSensingComponent;
class UParasiteSoundManagerComponent;

UENUM( BlueprintType )
enum class EParasiteCinematicMode : uint8
{
	None,
	EatAnimation,
	RushPlayer,
};

UCLASS( Abstract )
class METROXHUNTER_API AParasite : public ACharacter, public IHealthHolder, public IEQSContextProvider
{
	GENERATED_BODY()

public:
	AParasite();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	virtual void Landed( const FHitResult& Hit ) override;

	// Begin IHealthHolder interface
	bool TakeDamage_Implementation( UPARAM( ref ) FDamageContext& DamageContext );
	bool IsShownAsDamageableToPlayer_Implementation( EDamageType DamageType ) override;
	// End IHealthHolder interface

	// Begin IEQSContextProvider interface
	FVector GetEQSStartLocation_Implementation() const override;
	AActor* GetEQSTargetActor_Implementation() const override;
	// End IEQSContextProvider interface

	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void UpdateDataAsset();

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Parasite" )
	void StartPossessingCorpse( APossessableCorpse* Corpse );
	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void PossessCorpse( APossessableCorpse* Corpse );

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Parasite" )
	void StartEnteringVent( AVent* Vent );
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Parasite" )
	void StartExitingVent( AVent* Vent );

	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "Parasite" )
	void PrepareJumpAttack();
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "Parasite" )
	void EndPrepareJumpAttack();
	UFUNCTION( BlueprintPure, Category = "Parasite" )
	bool IsPreparingJump() const;

	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void JumpAttack();
	UFUNCTION( BlueprintPure, Category = "Parasite" )
	bool IsJumpAttacking() const;

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Parasite" )
	void DoBiteAttack( AActor* Target );

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "Parasite" )
	void EmitBloodSplash( const FDamageContext& DamageContext );
	UFUNCTION( BlueprintPure, Category = "Parasite" )
	bool HasEmittedBlood() const;

	UFUNCTION( BlueprintPure, Category = "Parasite" )
	float GetDefaultMoveSpeed() const;
	UFUNCTION( BlueprintPure, Category = "Parasite" )
	float GetFleeMoveSpeed() const;
	UFUNCTION( BlueprintPure, Category = "Parasite" )
	float GetRandomMeshScale() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( 
		FOnPossessCorpse, 
		AParasite*, Parasite, 
		APossessableCorpse*, Corpse,
		AZeroEnemy*, Enemy 
	);
	UPROPERTY( BlueprintAssignable, Category = "Parasite" )
	FOnPossessCorpse OnPossessCorpse {};

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Parasite" )
	UHealthComponent* HealthComponent = nullptr;
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Parasite" )
	UPawnSensingComponent* PawnSensingComponent = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "Parasite" )
	UParasiteSoundManagerComponent* SoundManagerComponent = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Parasite" )
	USaveLoadComponent* SaveComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Parasite", meta = ( ExposeOnSpawn = true ) )
	UParasiteData* DataAsset = nullptr;

	UPROPERTY( EditInstanceOnly, BlueprintReadOnly, Category = "Parasite" )
	bool bCanEverUseVents = true;

	UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = "Parasite", meta = ( ExposeOnSpawn = true ) )
	EParasiteCinematicMode CinematicMode = EParasiteCinematicMode::None;

private:
	UFUNCTION()
	void OnHit(
		AActor* SelfActor, AActor* OtherActor,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

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
	float DefaultMoveSpeed = 0.0f;
	float FleeMoveSpeed = 0.0f;
	float RandomMeshScale = 0.0f;

	bool bIsPreparingJump = false;
	bool bIsJumpAttacking = false;
	bool bHasAlreadyDamaged = false;
	bool bHasEmittedBlood = false;
};
