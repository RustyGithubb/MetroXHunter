/* 
 * Implemented by BARRAU Benoit 
 */

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IReloadSystemUpdate.generated.h"

UINTERFACE(Blueprintable)
class UIReloadSystemUpdate : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class METROXHUNTER_API IIReloadSystemUpdate
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Reload System")
	void UpdateReloadSystemState(EGunReloadState NewState);

};
