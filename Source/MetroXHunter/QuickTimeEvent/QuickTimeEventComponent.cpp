/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "QuickTimeEvent/QuickTimeEventComponent.h"
#include "QuickTimeEvent/QuickTimeEventData.h"

#include "PlayerController/PlayerInputHandler.h"

#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"

#include "Library/UtilityLibrary.h"
#include "Library/GameplayLibrary.h"

constexpr float PERCENT = 100.0f;

UQuickTimeEventComponent::UQuickTimeEventComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UQuickTimeEventComponent::BeginPlay()
{
	Super::BeginPlay();

	SetupPlayerInputComponent();
}

void UQuickTimeEventComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// Check event has started (e.g. event delay)
	float EventTime = GetEventTime();
	if ( EventTime < 0.0f ) return;

	// Check for dead zone time
	if ( bIsInDeadZone )
	{
		// Stop dead zone if we went over its time
		if ( EventTime >= DataAsset->DeadZoneTime )
		{
			DecreaseOffsetTime = EventTime;
			bIsInDeadZone = false;
		}
		else
		{
			return;
		}
	}

	// Decrease progress over time
	float ProgressDecrease = GetProgressDecreasePerSecond();
	InputProgress -= ProgressDecrease * DeltaTime;
	UE_VLOG(
		this, LogTemp, Verbose,
		TEXT( "QuickTimeEvent is decreasing by a progress of %f!" ), ProgressDecrease * PERCENT
	);
	UE_VLOG(
		this, LogTemp, Verbose,
		TEXT( "QuickTimeEvent is running with a progress of %f!" ), InputProgress
	);

	// Fail when maximum curve time is reached
	if ( DataAsset->bShouldFailAtMaxCurveTime && EventTime >= MaxCurveTime )
	{
		StopEvent( EQuickTimeEventResult::Failed );
	}
	// Fail when input progress is below the threshold for a minimum amount of time
	else if ( InputProgress < DataAsset->FailUnderProgress / PERCENT )
	{
		FailTime += DeltaTime;
		if ( FailTime >= DataAsset->FailUnderProgressTime )
		{
			StopEvent( EQuickTimeEventResult::Failed );
		}
	}
	// Success when input progress is above 100%
	else if ( InputProgress > 1.0f )
	{
		StopEvent( EQuickTimeEventResult::Succeed );
	}
}

void UQuickTimeEventComponent::StartEvent( UQuickTimeEventData* NewDataAsset, AActor* NewInflictor )
{
	verifyf( IsValid( NewDataAsset ), TEXT( "QuickTimeEvent must start with a valid DataAsset!" ) );

	DataAsset = NewDataAsset;
	InputProgress = DataAsset->StartProgress / PERCENT;
	FailTime = 0.0f;

	Inflictor = NewInflictor;
	bIsInDeadZone = true;
	EventStartTime = GetWorld()->GetTimeSeconds() + DataAsset->EventDelay;

	float MinTime = 0.0f;
	DataAsset->ProgressDecreaseCurve->GetTimeRange( MinTime, MaxCurveTime );

	// Switch to quick time event's input mapping context
	if ( IsValid( PlayerController ) )
	{
		verify( !InputMappingContext.IsNull() );
		IPlayerInputHandler::Execute_SetInputMappingContext(
			PlayerController,
			InputMappingContext.LoadSynchronous()
		);

		InputComponent->BindAction(
			DataAsset->InputAction.LoadSynchronous(), ETriggerEvent::Started,
			this, &UQuickTimeEventComponent::OnInput
		);

		UUtilityLibrary::PrintWarning(
			TEXT( "QTE: Start event, HasMappingContext=%d, InputAction=%s" ),
			PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()->HasMappingContext( InputMappingContext.LoadSynchronous() ),
			*GetNameSafe( DataAsset->InputAction.LoadSynchronous() )
		);
	}

	SetComponentTickEnabled( true );

	OnEventStarted.Broadcast( this, DataAsset );

	UE_VLOG( this, LogTemp, Verbose, TEXT( "QuickTimeEvent is starting!" ) );
}

void UQuickTimeEventComponent::StopEvent( EQuickTimeEventResult EventResult )
{
	SetComponentTickEnabled( false );

	if ( IsValid( PlayerController ) )
	{
		verify( !InputMappingContext.IsNull() );
		IPlayerInputHandler::Execute_RevertInputMappingContext(
			PlayerController,
			InputMappingContext.LoadSynchronous()
		);

		InputComponent->ClearBindingsForObject( this );
	}

	Result = EventResult;
	OnEventStopped.Broadcast( this, DataAsset, Result );

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
	
	float Time = GetEventTime() - DecreaseOffsetTime;
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

AActor* UQuickTimeEventComponent::GetEventInflictor() const
{
	return Inflictor;
}

float UQuickTimeEventComponent::GetInputProgress() const
{
	return InputProgress;
}

void UQuickTimeEventComponent::SetupPlayerInputComponent()
{
	PlayerController = UGameplayLibrary::GetPlayerControllerChecked( GetOwner() );
	InputComponent = CastChecked<UEnhancedInputComponent>( PlayerController->InputComponent );
}

void UQuickTimeEventComponent::OnInput( const FInputActionInstance& InputInstance )
{
	UUtilityLibrary::PrintMessage(
		TEXT( "QTE Input: %s need %s (EventTime %f)" ), 
		*GetNameSafe( InputInstance.GetSourceAction() ),
		*GetNameSafe( DataAsset->InputAction.Get() ),
		GetEventTime()
	);

	// Check that the event actually started
	if ( GetEventTime() < 0.0f ) return;

	// Compare current input to the one from DataAsset
	if ( InputInstance.GetSourceAction() != DataAsset->InputAction.Get() ) return;

	InputProgress += DataAsset->ProgressPerInput / 100.0f;
	UUtilityLibrary::PrintMessage( TEXT( "Input %f (%f)!" ), InputProgress, DataAsset->ProgressPerInput / 100.0f );

	// Manually stop dead zone when a correct input is received
	if ( bIsInDeadZone )
	{
		DecreaseOffsetTime = GetEventTime();
		bIsInDeadZone = false;
	}
}
