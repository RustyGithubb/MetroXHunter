/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Parasite.generated.h"

class AZeroEnemy;
class APossessableCorpse;

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
		FOnPossess, 
		AParasite*, Parasite, 
		APossessableCorpse*, Corpse,
		AZeroEnemy*, Enemy 
	);
	UPROPERTY( BlueprintAssignable, Category = "Parasite" )
	FOnPossess OnPossess {};
};
