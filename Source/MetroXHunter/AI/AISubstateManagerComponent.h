/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AISubstateManagerComponent.generated.h"

class UAISubstate;

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UAISubstateManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAISubstateManagerComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent( 
		float DeltaTime, 
		ELevelTick TickType, 
		FActorComponentTickFunction* ThisTickFunction 
	) override;

	UFUNCTION( BlueprintCallable, Category = "AISubstateManager" )
	void CreateSubstates( const TArray<TSubclassOf<UAISubstate>> SubstateClasses );

	UFUNCTION( BlueprintCallable, Category = "AISubstateManager" )
	void SwitchToSubstate( int32 Index );

public:
	//UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "AISubstateManager" )

	//UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "AISubstateManager" )

private:
	TArray<UAISubstate*> Substates;
	UAISubstate* CurrentSubstate = nullptr;

	int32 CurrentIndex = 0;
};
