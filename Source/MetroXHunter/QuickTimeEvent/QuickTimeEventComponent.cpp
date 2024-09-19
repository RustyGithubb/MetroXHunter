/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "QuickTimeEvent/QuickTimeEventComponent.h"
#include "QuickTimeEvent/QuickTimeEventData.h"

#include "Engine/LatentActionManager.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
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

	float ProgressDecrease = GetProgressDecreasePerSecond();
	InputProgress -= ProgressDecrease * DeltaTime;
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

	AddInputMappingContext();

	SetComponentTickEnabled( true );

	EventStartTime = GetWorld()->GetTimeSeconds();
	OnEventStarted.Broadcast();

	UE_VLOG( this, LogTemp, Verbose, TEXT( "QuickTimeEvent is starting!" ) );
}

void UQuickTimeEventComponent::StopEvent( EQuickTimeEventResult EventResult )
{
	SetComponentTickEnabled( false );

	RemoveInputMappingContext();

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

float UQuickTimeEventComponent::GetInputProgress() const
{
	return InputProgress;
}

void UQuickTimeEventComponent::SetupPlayerInputComponent()
{
	UInputComponent* PlayerInputComponent = PlayerController->InputComponent;
	
	verify( !InputAction.IsNull() );

	// Set up action bindings
	auto EnhancedInputComponent = CastChecked<UEnhancedInputComponent>( PlayerInputComponent );
	EnhancedInputComponent->BindAction(
		InputAction.LoadSynchronous(), ETriggerEvent::Started,
		this, &UQuickTimeEventComponent::OnInput
	);
}

void UQuickTimeEventComponent::AddInputMappingContext()
{
	verify( !InputMappingContext.IsNull() );

	auto LocalPlayer = PlayerController->GetLocalPlayer();
	auto InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	InputSystem->AddMappingContext( 
		InputMappingContext.LoadSynchronous(), 
		InputMappingContextPriority 
	);

	if ( !DefaultInputMappingContext.IsNull() )
	{
		InputSystem->RemoveMappingContext( DefaultInputMappingContext.LoadSynchronous() );
	}
}

void UQuickTimeEventComponent::RemoveInputMappingContext()
{
	verify( !InputMappingContext.IsNull() );

	auto LocalPlayer = PlayerController->GetLocalPlayer();
	auto InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	InputSystem->RemoveMappingContext( InputMappingContext.LoadSynchronous() );
	
	if ( !DefaultInputMappingContext.IsNull() )
	{
		InputSystem->AddMappingContext( DefaultInputMappingContext.LoadSynchronous(), 0 );
	}
}

void UQuickTimeEventComponent::OnInput()
{
	// TODO: Avoid this by using a specific InputMappingContext for QTE inputs
	if ( DataAsset == nullptr ) return;

	InputProgress += DataAsset->ProgressPerInput / 100.0f;
}
