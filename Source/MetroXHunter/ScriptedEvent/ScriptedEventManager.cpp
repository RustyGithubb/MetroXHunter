/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "ScriptedEvent/ScriptedEventManager.h"

#include "Components/BillboardComponent.h"

#include "Library/UtilityLibrary.h"



void UScriptedEvent::RunScriptedEvent( AScriptedEventManager* InManager )
{
	Manager = InManager;

	if ( OnRunScriptedEvent() )
	{
		OnEventFinished.Broadcast( this );
	}
}

UWorld* UScriptedEvent::GetWorld() const
{
	if ( !IsValid( Manager ) ) return nullptr;
	return Manager->GetWorld();
}

bool UScriptedEvent::OnRunScriptedEvent_Implementation()
{
	UUtilityLibrary::PrintWarning(
		TEXT( "ScriptedEvent: %s is not implemented" ),
		*GetName()
	);

	return true;
}


AScriptedEventManager::AScriptedEventManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>( TEXT( "RootComponent" ) );
	RootComponent = SceneComponent;

	IconComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>( TEXT( "IconComponent" ) );
	if ( IconComponent != nullptr )
	{
		IconComponent->SetupAttachment( RootComponent );

		IconComponent->SetRelativeScale3D( FVector { 0.75f, 0.75f, 0.75f } );
		IconComponent->Sprite = ConstructorHelpers::FObjectFinderOptional<UTexture2D>(
			TEXT( "/Engine/EditorResources/S_Note" )
		).Get();
	}
}

void AScriptedEventManager::BeginPlay()
{
	Super::BeginPlay();

	SortScriptedEventsByStartTime();

	for ( UScriptedEvent* ScriptedEvent : ScriptedEvents )
	{
		ScriptedEvent->OnEventFinished.AddDynamic( this, &AScriptedEventManager::OnEventFinished );
	}
}

void AScriptedEventManager::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Check index is not out-of-bounds, meaning the event is finished
	if ( CurrentEventIndex >= ScriptedEvents.Num() )
	{
		SetActorTickEnabled( false );
		OnRunEventsEnded.Broadcast( this );
		return;
	}

	// Increase event time
	CurrentEventTime += DeltaTime;

	// Check current event is valid (not set to None)
	UScriptedEvent* CurrentEvent = ScriptedEvents[CurrentEventIndex];
	if ( !IsValid( CurrentEvent ) )
	{
		CurrentEventIndex++;
		return;
	}

	// Check current event time
	if ( CurrentEventTime < CurrentEvent->EventStartTime ) return;

	CurrentEvent->RunScriptedEvent( this );
	CurrentEventIndex++;
}

void AScriptedEventManager::RunScriptedEvents()
{
	CurrentEventTime = 0.0f;
	CurrentEventIndex = 0;
	CurrentFinishedEvents = 0;

	SetActorTickEnabled( true );

	OnRunEventsStarted.Broadcast( this );
}

void AScriptedEventManager::SortScriptedEventsByStartTime()
{
	// Sort all events by start time to use an increasing index to loop over our events
	ScriptedEvents.Sort(
		[]( const UScriptedEvent& A, const UScriptedEvent& B )
		{
			return A.EventStartTime < B.EventStartTime;
		}
	);
}

void AScriptedEventManager::OnEventFinished( UScriptedEvent* ScriptedEvent )
{
	CurrentFinishedEvents++;

	if ( CurrentFinishedEvents == ScriptedEvents.Num() )
	{
		OnEventsFinished.Broadcast( this );
	}
}
