#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GeneralInteractable.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USphereComponent;
class UWidgetComponent;
class UInteractableWidget;
class UInteractableComponent;
class UInteractionComponent;

/*
 * Parent class of all Interactables
 *
 * Important: You will have to set the widget class in blueprint to WBP_Interactable for all the child class !
 */
UCLASS()
class METROXHUNTER_API AGeneralInteractable : public AActor
{
	GENERATED_BODY()

public:
	AGeneralInteractable();

	virtual void BeginPlay() override;

	void BindToDelegates();

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

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Interactable|UI" )
	UWidgetComponent* Widget = nullptr;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Interactable|UI" )
	UInteractableWidget* InteractableWidget = nullptr;

protected:
	UFUNCTION()
	virtual void Interact() {};

private:
	UFUNCTION()
	void OnInnerCircleOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnOutterCircleOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult
	);

	UFUNCTION()
	virtual void OnInnerCircleOverlapEnd(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
	);

	UFUNCTION()
	void OnOutterCircleOverlapEnd(
		UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex
	);

	UFUNCTION()
	void OnInteractableTargeted();

	UFUNCTION()
	void OnInteractableUntargeted();

private:
	UInteractionComponent* PlayerInteractionComponent = nullptr;
};
