#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

enum class E_InteractionType : uint8;
class UInteractionComponent;

/*
 * The Interactable Component owns the interaction logic and needs to be attached to the interactable.
 */
UCLASS( BlueprintType, meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void OnPlayerOverlap( UInteractionComponent* InteractionComponent );
	void OnPlayerOut( UInteractionComponent* InteractionComponent );

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnInteract );
	UPROPERTY( BlueprintCallable, Category = "Interaction|Event" )
	FOnInteract OnInteract;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnTargeted );
	UPROPERTY( BlueprintCallable, Category = "Interaction|Event" )
	FOnTargeted OnTargeted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnUntargeted );
	UPROPERTY( BlueprintCallable, Category = "Interaction|Event" )
	FOnUntargeted OnUntargeted;

public:
	AActor* Owner = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Interaction" )
	E_InteractionType InteractionType;
};
