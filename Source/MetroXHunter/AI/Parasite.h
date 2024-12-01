/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AI/ParasiteData.h"
#include "Parasite.generated.h"

class AZeroEnemy;
class APossessableCorpse;
class AVent;

class UHealthComponent;
class UPawnSensingComponent;

UCLASS( Abstract )
class METROXHUNTER_API AParasite : public ACharacter
{
	GENERATED_BODY()

public:
	AParasite();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	virtual void Landed( const FHitResult& Hit ) override;

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
	/*UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void EnterVent( AVent* Vent );*/

	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void JumpAttack();
	UFUNCTION( BlueprintPure, Category = "Parasite" )
	bool IsJumpAttacking() const;

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

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Parasite" )
	UParasiteData* DataAsset = nullptr;

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
	bool bIsJumpAttacking = false;
};
