#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

enum class E_InteractionType : uint8;
class UInteractionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTargeted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUntargeted);

UCLASS( Blueprintable, meta = ( ABSTRACT ) )
class METROXHUNTER_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractableComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	AActor* Owner = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Interaction" )
	E_InteractionType InteractionType;

	void OnPlayerOverlap(UInteractionComponent* InteractionComponent);
	void OnPlayerOut(UInteractionComponent* InteractionComponent );

	UPROPERTY( BlueprintCallable, Category = "Interaction|Event" )
	FOnInteract OnInteract;

	UPROPERTY( BlueprintCallable, Category = "Interaction|Event" )
	FOnTargeted OnTargeted;

	UPROPERTY( BlueprintCallable, Category = "Interaction|Event" )
	FOnUntargeted OnUntargeted;
};
