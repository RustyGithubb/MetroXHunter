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
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( Units = "Seconds" ) )
	float EventStartTime = 0.0f;

	AScriptedEventManager* Manager = nullptr;

protected:
	UFUNCTION( BlueprintNativeEvent )
	void OnRunScriptedEvent();
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
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEventStarted, AScriptedEventManager*, Manager );
	UPROPERTY( BlueprintAssignable, Category = "ScriptedEventManager" )
	FOnEventStarted OnEventStarted {};

	/*
	 * Called when the scripted event sequence has just ended its execution.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnEventEnded, AScriptedEventManager*, Manager );
	UPROPERTY( BlueprintAssignable, Category = "ScriptedEventManager" )
	FOnEventEnded OnEventEnded {};

public:
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ScriptedEventManager" )
	USceneComponent* SceneComponent = nullptr;

	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "ScriptedEventManager" )
	UBillboardComponent* IconComponent = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "ScriptedEventManager" )
	TArray<UScriptedEvent*> ScriptedEvents {};

private:
	float CurrentEventTime = 0.0f;
	int32 CurrentEventIndex = 0;
};
