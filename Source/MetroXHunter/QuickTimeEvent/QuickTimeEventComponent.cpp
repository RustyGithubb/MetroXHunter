/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "QuickTimeEvent/QuickTimeEventComponent.h"
#include "QuickTimeEvent/QuickTimeEventData.h"

#include "Engine/LatentActionManager.h"

#include "EnhancedInputComponent.h"

constexpr float PERCENT = 100.0f;

UQuickTimeEventComponent::UQuickTimeEventComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UQuickTimeEventComponent::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = GetOwner()->GetInstigator()->GetLocalViewingPlayerController();
	SetupPlayerInputComponent();
}

void UQuickTimeEventComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	InputProgress -= DataAsset->ProgressDecreasePerSecond / PERCENT * DeltaTime;
	UE_VLOG(
		this, LogTemp, Verbose,
		TEXT( "QuickTimeEvent is running with a progress of %f!" ), InputProgress
	);

	if ( InputProgress < DataAsset->FailUnderProgress / PERCENT )
	{
		StopEvent( EQuickTimeEventResult::Failed );
	}
	else if ( InputProgress > 1.0f )
	{
		StopEvent( EQuickTimeEventResult::Succeed );
	}
}

void UQuickTimeEventComponent::StartEvent( UQuickTimeEventData* NewDataAsset )
{
	verifyf( IsValid( NewDataAsset ), TEXT( "QuickTimeEvent must start with a valid DataAsset!" ) );

	DataAsset = NewDataAsset;
	InputProgress = DataAsset->StartProgress / PERCENT;

	PlayerController->GetPawn()->DisableInput( PlayerController );
	PlayerController->DisableInput( PlayerController );

	SetComponentTickEnabled( true );

	OnEventStarted.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "QuickTimeEvent is starting!" ) );
}

void UQuickTimeEventComponent::StopEvent( EQuickTimeEventResult EventResult )
{
	SetComponentTickEnabled( false );

	PlayerController->GetPawn()->EnableInput( PlayerController );
	PlayerController->EnableInput( PlayerController );

	Result = EventResult;
	OnEventStopped.Broadcast( Result );

	UE_VLOG( this, LogTemp, Verbose, TEXT( "QuickTimeEvent has stopped!" ) );
}

void UQuickTimeEventComponent::LatentQuickTimeEvent(
	UObject* WorldContext,
	FLatentActionInfo LatentInfo,
	EQuickTimeEventOutputPins& OutputPins,
	UQuickTimeEventComponent* Component,
	UQuickTimeEventData* DataAsset
)
{
	// Get world from context object (asserting)
	UWorld* World = GEngine->GetWorldFromContextObjectChecked( WorldContext );

	// Get latent action manager from the world
	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	// Try to retrieve an already existing action
	auto ExistingAction = LatentActionManager.FindExistingAction<FLatentQuickTimeEvent>(
		LatentInfo.CallbackTarget, LatentInfo.UUID
	);
	if ( ExistingAction != nullptr ) return;

	// Create a new action
	auto Action = new FLatentQuickTimeEvent( LatentInfo, OutputPins, Component, DataAsset );
	LatentActionManager.AddNewAction( LatentInfo.CallbackTarget, LatentInfo.UUID, Action );
}

bool UQuickTimeEventComponent::IsEventRunning() const
{
	return IsComponentTickEnabled();
}

EQuickTimeEventResult UQuickTimeEventComponent::GetEventResult() const
{
	return Result;
}

float UQuickTimeEventComponent::GetInputProgress() const
{
	return InputProgress;
}

void UQuickTimeEventComponent::SetupPlayerInputComponent()
{
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;
	
	verifyf(
		InputAction != nullptr,
		TEXT( "QuickTimeEvent must have InputActions set up!" )
	);

	// Set up action bindings
	if ( UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>( PlayerInputComponent ) )
	{
		EnhancedInputComponent->BindAction(
			InputAction, ETriggerEvent::Started, 
			this, &UQuickTimeEventComponent::OnInput
		);
	}
}

void UQuickTimeEventComponent::OnInput()
{
	// TODO: Avoid this by using a specific InputMappingContext for QTE inputs
	if ( DataAsset == nullptr ) return;

	InputProgress += DataAsset->ProgressPerInput / 100.0f;
}

void FLatentQuickTimeEvent::UpdateOperation( FLatentResponse& Response )
{
	// Start the quick time event
	if ( !bIsInitialized )
	{
		bIsInitialized = true;

		Component->StartEvent( DataAsset );
		return;
	}

	// Wait for the event to stop
	if ( Component->IsEventRunning() ) return;

	// Look for the result of the event
	switch ( Component->GetEventResult() )
	{
		case EQuickTimeEventResult::Succeed:
		{
			OutputPins = EQuickTimeEventOutputPins::OnSucceed;
			break;
		}
		case EQuickTimeEventResult::Failed:
		{
			OutputPins = EQuickTimeEventOutputPins::OnFailed;
			break;
		}
	}

	// Trigger the corresponding output pin and terminate
	Response.FinishAndTriggerIf( 
		true, 
		LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget 
	);
}
