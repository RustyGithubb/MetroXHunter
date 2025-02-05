/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScriptedEventManager.generated.h"

class UBillboardComponent;
class AScriptedEventManager;

/*
 * Object representing a single scripted event that can be run via a ScriptedEventManager actor.
 * This is an abstract class.
 */
UCLASS( Abstract, BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew, CollapseCategories, Within = "ScriptedEventManager" )
class METROXHUNTER_API UScriptedEvent : public UObject
{
	GENERATED_BODY()

public:
	void RunScriptedEvent( AScriptedEventManager* Manager );

	UWorld* GetWorld() const override;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEventFinished, UScriptedEvent*, ScriptedEvent );
	UPROPERTY( BlueprintAssignable, Category = "ScriptedEventManager" )
	FOnEventFinished OnEventFinished {};

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( Units = "Seconds" ) )
	float EventStartTime = 0.0f;

	AScriptedEventManager* Manager = nullptr;

protected:
	UFUNCTION( BlueprintNativeEvent )
	bool OnRunScriptedEvent();
};

/*
 * Actor responsible for holding a list of ScriptedEvent that can be run at anytime.
 * This is mainly used by our TriggerBoxEvent actor created in Blueprint.
 */
UCLASS()
class METROXHUNTER_API AScriptedEventManager : public AActor
{
	GENERATED_BODY()

public:
	AScriptedEventManager();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaTime ) override;

	UFUNCTION( BlueprintCallable, Category = "ScriptedEventManager" )
	void RunScriptedEvents();

	UFUNCTION( BlueprintCallable, CallInEditor, Category = "ScriptedEventManager", meta = ( DisplayName = "Sort by Start Time" ) )
	void SortScriptedEventsByStartTime();

public:
	/*
	 * Called when the scripted event sequence has just started its execution.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnStartedRunEvents, AScriptedEventManager*, Manager );
	UPROPERTY( BlueprintAssignable, Category = "ScriptedEventManager" )
	FOnStartedRunEvents OnRunEventsStarted {};

	/*
	 * Called when the scripted event sequence has just ended its execution.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEndedRunEvents, AScriptedEventManager*, Manager );
	UPROPERTY( BlueprintAssignable, Category = "ScriptedEventManager" )
	FOnEndedRunEvents OnRunEventsEnded {};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnFinishedEvents, AScriptedEventManager*, Manager );
	UPROPERTY( BlueprintAssignable, Category = "ScriptedEventManager" )
	FOnFinishedEvents OnEventsFinished {};

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ScriptedEventManager" )
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ScriptedEventManager" )
	UBillboardComponent* IconComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "ScriptedEventManager" )
	TArray<UScriptedEvent*> ScriptedEvents {};

private:
	UFUNCTION()
	void OnEventFinished( UScriptedEvent* ScriptedEvent );

private:
	float CurrentEventTime = 0.0f;
	int32 CurrentEventIndex = 0;
	int32 CurrentFinishedEvents = 0;
};
