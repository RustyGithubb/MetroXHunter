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
