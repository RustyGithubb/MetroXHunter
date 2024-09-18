/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuickTimeEventComponent.generated.h"

class UQuickTimeEventData;
class UInputAction;

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UQuickTimeEventComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuickTimeEventComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	void StartEvent( UQuickTimeEventData* NewDataAsset );
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	void StopEvent();

	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	bool IsEventRunning() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	float GetInputProgress() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnEventStarted );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnEventStarted OnEventStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnEventStopped );
	UPROPERTY( BlueprintAssignable, Category = "ZeroEnemy" )
	FOnEventStopped OnEventStopped;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent" )
	UInputAction* InputAction = nullptr;

private:
	void SetupPlayerInputComponent();

	void OnInput();

private:
	APlayerController* PlayerController = nullptr;

	UQuickTimeEventData* DataAsset = nullptr;
	float InputProgress = 0.0f;
};
