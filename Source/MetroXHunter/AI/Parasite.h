/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Parasite.generated.h"

class AZeroEnemy;
class APossessableCorpse;
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

	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "Parasite" )
	void StartPossessing( APossessableCorpse* Corpse );
	UFUNCTION( BlueprintCallable, Category = "Parasite" )
	void Possess( APossessableCorpse* Corpse );

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
};
