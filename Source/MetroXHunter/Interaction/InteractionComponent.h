// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionComponent.generated.h"

class UInteractableComponent;

UCLASS( BlueprintType, meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractionComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void AddNearInteractable( UInteractableComponent* InInteractable );
	void RemoveNearInteractable( UInteractableComponent* InInteractable );

private:
	void GetReferences();
	void RetrieveClosestInteractable();
	void UpdateViewport();

	APlayerController* PlayerController = nullptr;

	/*
	* The closest interactable from the player's view
	*/
	UInteractableComponent* CurrentInteractable = nullptr;

	/*
	* All the interactables near the player
	*/
	TArray<UInteractableComponent*> NearInteractables;

	/*
	* If there is any interactable near the player
	*/
	bool bIsNearInteractable = false;
};
