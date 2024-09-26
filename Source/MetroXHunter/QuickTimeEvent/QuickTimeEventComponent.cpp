/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "QuickTimeEvent/QuickTimeEventComponent.h"
#include "QuickTimeEvent/QuickTimeEventData.h"

#include "PlayerController/PlayerInputHandler.h"

#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"

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

	// Decrease progress over time
	float ProgressDecrease = GetProgressDecreasePerSecond();
	InputProgress -= ProgressDecrease * DeltaTime;
	UE_VLOG(
		this, LogTemp, Verbose,
		TEXT( "QuickTimeEvent is running with a progress of %f!" ), InputProgress
	);

	// Check for fail condition
	if ( InputProgress < DataAsset->FailUnderProgress / PERCENT )
	{
		StopEvent( EQuickTimeEventResult::Failed );
	}
	// Check for success condition
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

	// Switch to quick time event's input mapping context
	verify( !InputMappingContext.IsNull() );
	IPlayerInputHandler::Execute_SetInputMappingContext( PlayerController, InputMappingContext.LoadSynchronous() );

	SetComponentTickEnabled( true );

	EventStartTime = GetWorld()->GetTimeSeconds();
	OnEventStarted.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "QuickTimeEvent is starting!" ) );
}

void UQuickTimeEventComponent::StopEvent( EQuickTimeEventResult EventResult )
{
	SetComponentTickEnabled( false );

	IPlayerInputHandler::Execute_ResetInputMappingContext( PlayerController );

	Result = EventResult;
	OnEventStopped.Broadcast( Result );

	UE_VLOG( this, LogTemp, Verbose, TEXT( "QuickTimeEvent has stopped!" ) );
}

bool UQuickTimeEventComponent::IsEventRunning() const
{
	return IsComponentTickEnabled();
}

float UQuickTimeEventComponent::GetEventTime() const
{
	return GetWorld()->GetTimeSeconds() - EventStartTime;
}

float UQuickTimeEventComponent::GetProgressDecreasePerSecond() const
{
	auto Curve = DataAsset->ProgressDecreaseCurve;
	if ( Curve == nullptr )
	{
		return DataAsset->ProgressDecreasePerSecond / PERCENT;
	}
	
	float Time = GetEventTime();
	return Curve->GetFloatValue( Time ) / PERCENT;
}

EQuickTimeEventResult UQuickTimeEventComponent::GetEventResult() const
{
	return Result;
}

UQuickTimeEventData* UQuickTimeEventComponent::GetEventDataAsset() const
{
	return DataAsset;
}

float UQuickTimeEventComponent::GetInputProgress() const
{
	return InputProgress;
}

void UQuickTimeEventComponent::SetupPlayerInputComponent()
{
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;
	
	verify( !InputMappingContext.IsNull() );

	// Retrieve all mappings of the InputMappingContext
	auto& InputMappings = InputMappingContext.LoadSynchronous()->GetMappings();
	verifyf( !InputMappings.IsEmpty(), TEXT( "The InputMappingContext must not be empty!" ) );

	// Set up action bindings
	auto EnhancedInputComponent = CastChecked<UEnhancedInputComponent>( PlayerInputComponent );
	for ( auto& InputMapping : InputMappings )
	{
		EnhancedInputComponent->BindAction(
			InputMapping.Action, ETriggerEvent::Started,
			this, &UQuickTimeEventComponent::OnInput
		);
	}
}

void UQuickTimeEventComponent::OnInput( const FInputActionInstance& InputInstance )
{
	// Compare current input to the one from DataAsset
	if ( InputInstance.GetSourceAction() != DataAsset->InputAction ) return;

	InputProgress += DataAsset->ProgressPerInput / 100.0f;
}
