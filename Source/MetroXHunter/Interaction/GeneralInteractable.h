#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeneralInteractable.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USphereComponent;
class UWidgetComponent;
class UInteractableComponent;

UCLASS()
class METROXHUNTER_API AGeneralInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	AGeneralInteractable();

	virtual void BeginPlay() override;
//	virtual void Tick(float DeltaTime) override;

public:

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Interactable" )
	UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Interactable" )
	USceneComponent* SceneRoot = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Interactable|Collisions" )
	USphereComponent* InnerCollision = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Interactable|Collisions" )
	USphereComponent* OutterCollision = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Interactable|Component" )
	UInteractableComponent* InteractableComponent = nullptr;
};
