/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Components/ActorComponent.h"
#include "QuickTimeEventComponent.generated.h"

class UQuickTimeEventData;
class UInputAction;

/*
 * Represent the result of the quick time event.
 */
UENUM( BlueprintType )
enum class EQuickTimeEventResult : uint8
{
	/*
	 * The player succeed the event.
	 */
	Succeed,
	/*
	 * The player failed the event.
	 */
	Failed,
};

/*
 * Component responsible for the quick time event system.
 * Designed to be added on the player character as it needs its controller to bind the inputs.
 */
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
	void StopEvent( EQuickTimeEventResult EventResult );

	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	bool IsEventRunning() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	EQuickTimeEventResult GetEventResult() const;
	UFUNCTION( BlueprintCallable, Category = "QuickTimeEvent" )
	float GetInputProgress() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnEventStarted );
	UPROPERTY( BlueprintAssignable, Category = "QuickTimeEvent" )
	FOnEventStarted OnEventStarted;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEventStopped, EQuickTimeEventResult, EventResult );
	UPROPERTY( BlueprintAssignable, Category = "QuickTimeEvent" )
	FOnEventStopped OnEventStopped;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "QuickTimeEvent" )
	UInputAction* InputAction = nullptr;

private:
	void SetupPlayerInputComponent();

	void OnInput();

private:
	EQuickTimeEventResult Result = EQuickTimeEventResult::Succeed;
	APlayerController* PlayerController = nullptr;

	UQuickTimeEventData* DataAsset = nullptr;
	float InputProgress = 0.0f;
};