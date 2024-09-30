/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIAttackerComponent.generated.h"

class UAITargetComponent;

/*
 *
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UAIAttackerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIAttackerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay( const EEndPlayReason::Type EndPlayReason );

	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	bool ReserveTokens( UAITargetComponent* Target, int32 Tokens );
	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	bool FreeTokens( int32 Tokens = 0 );

	UFUNCTION( BlueprintCallable, Category = "AIAttacker" )
	UAITargetComponent* GetCurrentTarget() const;

private:
	UAITargetComponent* CurrentTarget = nullptr;
};
