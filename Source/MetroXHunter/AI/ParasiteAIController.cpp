/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/ParasiteAIController.h"
#include "AI/Parasite.h"
#include "AI/AITargetComponent.h"
#include "AI/AIAttackerComponent.h"
#include "Vent/Vent.h"

#include "Library/ConvarLibrary.h"
#include "Library/UtilityLibrary.h"

#include "Perception/PawnSensingComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

constexpr auto ENEMY_KEYNAME = TEXT( "EnemyActor" );
constexpr auto IN_DANGER_KEYNAME = TEXT( "bInDanger" );
constexpr auto NEXT_VENT_TIME_KEYNAME = TEXT( "NextVentTime" );
constexpr auto GROUP_PLACE_RADIUS = TEXT( "GroupPlaceRadius" );
constexpr auto GROUP_PLACE_INDEX = TEXT( "GroupPlaceIndex" );
constexpr auto MIN_ATTACKERS_TO_ATTACK = TEXT( "MinAttackersToAttack" );
constexpr auto JUMP_ATTACK_TOKEN = TEXT( "JumpAttackToken" );
constexpr auto JUMP_ATTACK_TOKEN_COOLDOWN = TEXT( "JumpAttackTokenCooldown" );

// Set default FollowingComponent to CrowdFollowingComponent so they move around each other
AParasiteAIController::AParasiteAIController( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>( TEXT( "PathFollowingComponent" ) ) )
{
	AttackerComponent = CreateDefaultSubobject<UAIAttackerComponent>( TEXT( "AIAttackerComponent" ) );
}

void AParasiteAIController::BeginPlay()
{
	Super::BeginPlay();

	AttackerComponent->OnGroupPlaceChanged.AddDynamic(
		this, &AParasiteAIController::OnGroupPlaceChanged
	);
}

void AParasiteAIController::OnPossess( APawn* InPawn )
{
	CustomPawn = CastChecked<AParasite>( InPawn );
	const UParasiteData* DataAsset = CustomPawn->DataAsset;

	verifyf(
		RunBehaviorTree( BehaviorTree ),
		TEXT( "Behavior Tree of %s failed to run" ), *GetName()
	);

	// Bind to pawn's events
	CustomPawn->PawnSensingComponent->OnSeePawn.AddDynamic(
		this, &AParasiteAIController::OnSeePawn
	);
	CustomPawn->PawnSensingComponent->OnHearNoise.AddDynamic(
		this, &AParasiteAIController::OnHearNoise
	);

	// Disabling crowd simulation fixes a bug where AI can't move on navmesh; but it disables crowd 
	// simulation features so it's just a quick patch before finding the real source
	if ( DataAsset->bIsCrowdSimulationDisabled )
	{
		auto CrowdComponent = GetComponentByClass<UCrowdFollowingComponent>();
		CrowdComponent->SetCrowdSimulationState( ECrowdSimulationState::Disabled );
	}

	//	If coming out of vent, delay next vent usage for a while
	if ( CustomPawn->GetOwner<AVent>() )
	{
		SetNextVentTime( GetWorld()->GetTimeSeconds() + DataAsset->ExitVentNextUseCooldown );
	}

	// Update blackboard values with data asset
	Blackboard->SetValueAsInt( MIN_ATTACKERS_TO_ATTACK, DataAsset->MinAttackersToAttack );
	Blackboard->SetValueAsInt( JUMP_ATTACK_TOKEN, DataAsset->JumpAttackToken );
	Blackboard->SetValueAsFloat( JUMP_ATTACK_TOKEN_COOLDOWN, DataAsset->JumpAttackTokenCooldown );

	Super::OnPossess( InPawn );
}

void AParasiteAIController::SetEnemy( AActor* Enemy )
{
	if ( auto TargetComponent = Enemy->GetComponentByClass<UAITargetComponent>() )
	{
		AttackerComponent->SetCurrentTarget( TargetComponent );
		TargetComponent->DeclareAttacker( AttackerComponent );
	}

	Blackboard->SetValueAsObject( ENEMY_KEYNAME, Enemy );
}

AActor* AParasiteAIController::GetEnemy() const
{
	return Cast<AActor>( Blackboard->GetValueAsObject( ENEMY_KEYNAME ) );
}

void AParasiteAIController::SetInDanger( bool bInDanger )
{
	Blackboard->SetValueAsBool( IN_DANGER_KEYNAME, bInDanger );
}

bool AParasiteAIController::IsInDanger() const
{
	return Blackboard->GetValueAsBool( IN_DANGER_KEYNAME );
}

void AParasiteAIController::SetNextVentTime( float GameTime )
{
	Blackboard->SetValueAsFloat( NEXT_VENT_TIME_KEYNAME, GameTime );
}

#if ENABLE_VISUAL_LOG
void AParasiteAIController::GrabDebugSnapshot( FVisualLogEntry* Snapshot ) const
{
	Super::GrabDebugSnapshot( Snapshot );

	FVisualLogStatusCategory Category( TEXT( "ParasiteAI" ) );
	Category.Add(
		TEXT( "VelocityLength" ),
		FString::SanitizeFloat( CustomPawn->GetCharacterMovement()->Velocity.Length() )
	);
	Category.Add(
		TEXT( "MaxWalkSpeed" ),
		FString::SanitizeFloat( CustomPawn->GetCharacterMovement()->MaxWalkSpeed )
	);

	Snapshot->Status.Add( Category );
}
#endif

void AParasiteAIController::OnSeePawn( APawn* SeenPawn )
{
	// Panic with its fellow mates
	if ( auto Parasite = Cast<AParasite>( SeenPawn ) )
	{
		// No need to panic again if it's already in panic
		if ( IsInDanger() ) return;

		auto ParasiteAI = Cast<AParasiteAIController>( Parasite->GetController() );
		if ( !IsValid( ParasiteAI ) ) return;
		if ( !ParasiteAI->IsInDanger() ) return;

		SetEnemy( ParasiteAI->GetEnemy() );
		SetInDanger( true );
	}

	// Panic when player is seen
	if ( !UConvarLibrary::IsAIIgnorePlayerConvarEnabled() && SeenPawn->IsPlayerControlled() )
	{
		SetEnemy( SeenPawn );
		SetInDanger( true );
	}
}

void AParasiteAIController::OnHearNoise( APawn* HeardPawn, const FVector& Location, float Volume )
{
	// Panic when player is heard
	if ( !UConvarLibrary::IsAIIgnorePlayerConvarEnabled() && HeardPawn->IsPlayerControlled() )
	{
		SetEnemy( HeardPawn );
		SetInDanger( true );
	}
}

void AParasiteAIController::OnGroupPlaceChanged( int32 LastGroupIndex, int32 NewGroupIndex )
{
	Blackboard->SetValueAsInt( GROUP_PLACE_INDEX, NewGroupIndex );

	//	Ignore removed-from-group events
	if ( NewGroupIndex == -1 ) return;

	//	Randomize group place radius
	UAITargetComponent* Target = AttackerComponent->GetCurrentTarget();
	const FAITargetGroupSettings& Settings = Target->GetGroupSettings( NewGroupIndex );
	const float Radius = UUtilityLibrary::RandomInRange( Settings.PlaceDistance );
	Blackboard->SetValueAsFloat( GROUP_PLACE_RADIUS, Radius );
}
