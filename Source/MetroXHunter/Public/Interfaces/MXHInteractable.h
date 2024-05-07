// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MXHInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMXHInteractable : public UInterface
{
	GENERATED_BODY()
};

class METROXHUNTER_API IMXHInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Events" )
	void Interact();
};
