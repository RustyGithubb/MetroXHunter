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
	void CreateSubstates( const TArray<TSubclassOf<UAISubstate>>& SubstateClasses );

	UFUNCTION( BlueprintCallable, Category = "AISubstateManager" )
	bool SwitchToSubstate( int32 Index );

	UFUNCTION( BlueprintCallable, Category = "AISubstateManager" )
	float GetSubstateTime() const;

public:
	/*
	 * Event called when the current substate changed from one to another.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSubstateSwitched );
	UPROPERTY( BlueprintAssignable, Category = "AISubstateManager" )
	FOnSubstateSwitched OnSubstateSwitched;

	//UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, Category = "AISubstateManager" )

private:
	UPROPERTY()
	TArray<UAISubstate*> Substates;
	UAISubstate* CurrentSubstate = nullptr;

	float GameTimeSinceLastSwitch = 0.0f;

	int32 CurrentIndex = 0;
};
