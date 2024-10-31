/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/ZeroEnemyAIController.h"
#include "AI/ZeroEnemy.h"
#include "AI/AIAttackerComponent.h"
#include "AI/AITargetComponent.h"
#include "AI/AISubstateManagerComponent.h"

#include "UtilityLibrary.h"
#include "Library/ConvarLibrary.h"

#include "Navigation/CrowdFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/KismetSystemLibrary.h"

constexpr auto AI_KEYNAME = TEXT( "AIState" );
constexpr auto PAWN_STATE_KEYNAME = TEXT( "PawnState" );
constexpr auto TARGET_KEYNAME = TEXT( "TargetActor" );
constexpr auto RUSH_TOKENS_KEYNAME = TEXT( "RushTokens" );
constexpr auto RUSH_TOKEN_COOLDOWN_KEYNAME = TEXT( "RushTokenCooldown" );
constexpr auto MELEE_TOKENS_KEYNAME = TEXT( "MeleeTokens" );
constexpr auto MELEE_TOKEN_COOLDOWN_KEYNAME = TEXT( "MeleeTokenCooldown" );

// Set default FollowingComponent to CrowdFollowingComponent so they move around each other
AZeroEnemyAIController::AZeroEnemyAIController( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>( TEXT( "PathFollowingComponent" ) ) )
{
	AttackerComponent = CreateDefaultSubobject<UAIAttackerComponent>( TEXT( "AIAttackerComponent" ) );
	SubstateManagerComponent = CreateDefaultSubobject<UAISubstateManagerComponent>( TEXT( "AISubstateManagerComponent" ) );
}

void AZeroEnemyAIController::OnPossess( APawn* InPawn )
{
	CustomPawn = CastChecked<AZeroEnemy>( InPawn );
	CustomPawn->OnStun.AddDynamic( this, &AZeroEnemyAIController::OnStun );
	CustomPawn->OnUnStun.AddDynamic( this, &AZeroEnemyAIController::OnUnStun );
	CustomPawn->OnRush.AddDynamic( this, &AZeroEnemyAIController::OnRush );
	CustomPawn->OnUnRush.AddDynamic( this, &AZeroEnemyAIController::OnUnRush );
	CustomPawn->OnStateUpdate.AddDynamic( this, &AZeroEnemyAIController::OnStateUpdate );
	const UZeroEnemyData* DataAsset = CustomPawn->Data;

	verifyf( RunBehaviorTree( BehaviorTree ), TEXT( "Behavior Tree of %s failed to run" ), *GetName() );

	// NOTE: We remove the AISubstate system for now; need to test the enemy first 
	//InitializeAISubstateManager();

	// Disabling crowd simulation fixes a bug where AI can't move on navmesh; but it disables crowd 
	// simulation features so it's just a quick patch before finding the real source
	if ( DataAsset->bIsCrowdSimulationDisabled )
	{
		auto CrowdComponent = GetComponentByClass<UCrowdFollowingComponent>();
		CrowdComponent->SetCrowdSimulationState( ECrowdSimulationState::Disabled );
	}

	// Update blackboard values with data asset
	Blackboard->SetValueAsInt( RUSH_TOKENS_KEYNAME, DataAsset->RushTokens );
	Blackboard->SetValueAsFloat( RUSH_TOKEN_COOLDOWN_KEYNAME, DataAsset->RushTokenCooldown );
	Blackboard->SetValueAsInt( MELEE_TOKENS_KEYNAME, DataAsset->MeleeTokens );
	Blackboard->SetValueAsFloat( MELEE_TOKEN_COOLDOWN_KEYNAME, DataAsset->MeleeTokenCooldown );

	SetActorTickEnabled( true );

	Super::OnPossess( InPawn );
}

void AZeroEnemyAIController::OnUnPossess()
{
	// Remove all events from pawn
	CustomPawn->OnStun.RemoveDynamic( this, &AZeroEnemyAIController::OnStun );
	CustomPawn->OnUnStun.RemoveDynamic( this, &AZeroEnemyAIController::OnUnStun );
	CustomPawn->OnRush.RemoveDynamic( this, &AZeroEnemyAIController::OnRush );
	CustomPawn->OnUnRush.RemoveDynamic( this, &AZeroEnemyAIController::OnUnRush );
	CustomPawn->OnStateUpdate.RemoveDynamic( this, &AZeroEnemyAIController::OnStateUpdate );

	StopScreamTimer();

	CustomPawn = nullptr;
	SetActorTickEnabled( false );

	Super::OnUnPossess();
}

void AZeroEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AZeroEnemyAIController::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if ( UConvarLibrary::IsAIDebugConvarEnabled() )
	{
		TickDebugDraw();
	}
}

void AZeroEnemyAIController::CombatTarget( AActor* InTarget )
{
	SetTarget( InTarget );
	SetState( EZeroEnemyAIState::Target );
}

void AZeroEnemyAIController::SetState( EZeroEnemyAIState State )
{
	Blackboard->SetValueAsEnum( AI_KEYNAME, (uint8)State );

	switch ( State )
	{
		case EZeroEnemyAIState::Idle:
			StopScreamTimer();
			break;
		default:
			if ( !ScreamTimerHandle.IsValid() )
			{
				StartScreamTimer();
			}
			break;
	}
}

EZeroEnemyAIState AZeroEnemyAIController::GetState() const
{
	return (EZeroEnemyAIState)Blackboard->GetValueAsEnum( AI_KEYNAME );
}

void AZeroEnemyAIController::SetTarget( AActor* InTarget )
{
	if ( auto TargetComponent = InTarget->GetComponentByClass<UAITargetComponent>() )
	{
		AttackerComponent->SetCurrentTarget( TargetComponent );
		TargetComponent->DeclareAttacker( AttackerComponent );
	}

	Blackboard->SetValueAsObject( TARGET_KEYNAME, InTarget );

	UUtilityLibrary::PrintMessage(
		TEXT( "AI: '%s' targeting '%s'" ),
		*GetName(),
		*GetNameSafe( InTarget )
	);
}

AActor* AZeroEnemyAIController::GetTarget() const
{
	auto Target = Blackboard->GetValueAsObject( TARGET_KEYNAME );
	if ( !Target ) return nullptr;

	return CastChecked<AActor>( Target );
}

float AZeroEnemyAIController::GetMadnessLevel() const
{
	int32 SubstatesCount = SubstateManagerComponent->GetSubstatesCount();
	if ( SubstatesCount == 0 ) return 0.0f;

	int32 SubstateIndex = SubstateManagerComponent->GetSubstateIndex();
	return static_cast<float>( SubstateIndex ) / static_cast<float>( SubstatesCount - 1 );
}

#if ENABLE_VISUAL_LOG
void AZeroEnemyAIController::GrabDebugSnapshot( FVisualLogEntry* Snapshot ) const
{
	Super::GrabDebugSnapshot( Snapshot );

	FVisualLogStatusCategory Category( TEXT( "ZeroEnemyAI" ) );
	Category.Add(
		TEXT( "State" ),
		UEnum::GetValueAsString( GetState() )
	);
	Category.Add(
		TEXT( "VelocityLength" ),
		FString::SanitizeFloat( CustomPawn->GetCharacterMovement()->Velocity.Length() )
	);
	Category.Add(
		TEXT( "MaxWalkSpeed" ),
		FString::SanitizeFloat( CustomPawn->GetCharacterMovement()->MaxWalkSpeed )
	);
	Category.Add(
		TEXT( "MadnessLevel" ),
		FString::SanitizeFloat( GetMadnessLevel() )
	);

	Snapshot->Status.Add( Category );
}
#endif

void AZeroEnemyAIController::InitializeAISubstateManager()
{
	const int32 SubstateIndex = CustomPawn->SpawnSubstateClass != nullptr 
		? CustomPawn->Data->SubstateClasses.Find( CustomPawn->SpawnSubstateClass ) 
		: 0;

	//	NOTE: Isn't it the best conversion code?
	auto& SubstateClasses = reinterpret_cast<TArray<TSubclassOf<UAISubstate>>&>( CustomPawn->Data->SubstateClasses );
	SubstateManagerComponent->CreateSubstates( SubstateClasses );
	SubstateManagerComponent->SwitchToSubstate( SubstateIndex );

	SubstateManagerComponent->OnSubstateSwitched.AddDynamic( this, &AZeroEnemyAIController::OnSubstateSwitched );
}

void AZeroEnemyAIController::TickDebugDraw()
{
	AActor* Target = GetTarget();

	FNumberFormattingOptions NumberFormattingOptions {};
	NumberFormattingOptions.MaximumFractionalDigits = 0;

	FFormatNamedArguments Args {};
	Args.Add( TEXT( "Name" ), FText::FromString( GetName() ) );
	Args.Add( TEXT( "State" ), FText::FromString( UEnum::GetValueAsString( GetState() ) ) );
	Args.Add( TEXT( "Target" ), FText::FromString( GetNameSafe( Target ) ) );
	Args.Add( TEXT( "Health" ), FText::AsNumber( CustomPawn->HealthComponent->CurrentHealth ) );
	Args.Add( TEXT( "LeftBodyParts" ), FText::AsNumber( CustomPawn->LeftBodyPartsCount ) );
	Args.Add( TEXT( "StartBodyParts" ), FText::AsNumber( CustomPawn->GetStartingBodyPartsCount() ) );
	Args.Add( TEXT( "MaxWalkSpeed" ), CustomPawn->GetCharacterMovement()->MaxWalkSpeed );
	Args.Add( TEXT( "VelocityLength" ),
		FText::AsNumber(
			CustomPawn->GetCharacterMovement()->Velocity.Length(),
			&NumberFormattingOptions
		)
	);

	float DistanceFromTarget = 0.0f;
	if ( IsValid( Target ) )
	{
		DistanceFromTarget = FVector::Distance( Target->GetActorLocation(), CustomPawn->GetActorLocation() );
	}
	Args.Add( TEXT( "DistanceFromTarget" ), FText::AsNumber( DistanceFromTarget, &NumberFormattingOptions ) );

	const FText Text = FText::Format(
		FTextFormat::FromString(
			"Self: {Name}:\n"
			"State: {State}\n"
			"Target: {Target}\n"
			"Health: {Health}\n"
			"BodyParts: {LeftBodyParts}/{StartBodyParts}\n"
			"VelocityLength: {VelocityLength} cm/s\n"
			"MaxWalkSpeed: {MaxWalkSpeed} cm/s\n"
			"DistanceFromTarget: {DistanceFromTarget} cm\n"
		),
		Args
	);

	UKismetSystemLibrary::DrawDebugString(
		this,
		FVector::ZeroVector,
		Text.ToString(),
		CustomPawn,
		FLinearColor::White
	);
}

void AZeroEnemyAIController::OnScreamUpdate()
{
	CustomPawn->Scream();
	StartScreamTimer();
}

void AZeroEnemyAIController::StartScreamTimer()
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(
		ScreamTimerHandle,
		this, &AZeroEnemyAIController::OnScreamUpdate,
		UUtilityLibrary::RandomInRange( CustomPawn->Data->ScreamTimeRange )
	);
}

void AZeroEnemyAIController::StopScreamTimer()
{
	if ( !ScreamTimerHandle.IsValid() ) return;

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer( ScreamTimerHandle );
}

void AZeroEnemyAIController::OnStun()
{
	switch ( GetState() )
	{
		case EZeroEnemyAIState::RushAttack:
		case EZeroEnemyAIState::MeleeAttack:
		case EZeroEnemyAIState::SpitAttack:
			SetState( EZeroEnemyAIState::Chase );
			break;
	}
	//SetState( EZeroEnemyAIState::Stun );
}

void AZeroEnemyAIController::OnUnStun()
{
	//SetState( EZeroEnemyAIState::Chase );
}

void AZeroEnemyAIController::OnRush()
{
	//SetState( EZeroEnemyAIState::RushAttack );
}

void AZeroEnemyAIController::OnUnRush()
{
	//SetState( EZeroEnemyAIState::Chase );
}

void AZeroEnemyAIController::OnStateUpdate()
{
	EZeroEnemyState PawnState = CustomPawn->GetState();
	Blackboard->SetValueAsEnum( PAWN_STATE_KEYNAME, (uint8)PawnState );

	switch ( PawnState )
	{
		case EZeroEnemyState::RushAttack:
		case EZeroEnemyState::RushAttackResolve:
			// Disable substate manager component when rushing
			// It avoids changing substate during QTE
			SubstateManagerComponent->SetComponentTickEnabled( false );
			break;
		default:
			SubstateManagerComponent->SetComponentTickEnabled( true );
			break;
	}
}

void AZeroEnemyAIController::OnSubstateSwitched()
{
	CustomPawn->Stun( CustomPawn->Data->SubstateChangeAnimationTime, false );
	CustomPawn->Scream();
}
