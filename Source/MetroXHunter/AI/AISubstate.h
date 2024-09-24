/* 
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AISubstate.generated.h"

class UAISubstateManagerComponent;
	
UENUM( BlueprintType )
enum class EAISubstateSwitchOrder : uint8
{
	/*
	 * Order to stay to the current substate.
	 */
	Stay,
	/*
	 * Order to switch to the next substate.
	 */
	Next,
	/*
	 * Order to switch to the previous substate.
	 */
	Previous,
};

/**
 * 
 */
UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UAISubstate : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "AISubstate" )
	void OnSubstateInitialized();
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "AISubstate" )
	void OnSubstateEntered();
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "AISubstate" )
	void OnSubstateExited();

	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category = "AISubstate" )
	EAISubstateSwitchOrder TickSubstate( float DeltaTime );

	UWorld* GetWorld() const override;

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "AISubstate" )
	UAISubstateManagerComponent* Manager = nullptr;
};