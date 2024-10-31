/* 
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "AI/ZeroEnemyAISubstate.h"
#include "AI/AISubstateManagerComponent.h"
#include "AI/ZeroEnemy.h"
#include "AI/ZeroEnemyAIController.h"

void UZeroEnemyAISubstate::OnSubstateInitialized_Implementation()
{
	AIController = CastChecked<AZeroEnemyAIController>( Manager->GetOwner() );
}

void UZeroEnemyAISubstate::OnSubstateEntered_Implementation()
{
	FZeroEnemyModifiers Modifiers {};
	Modifiers.WalkSpeedMultiplier = DataAsset->WalkSpeed / AIController->CustomPawn->Data->WalkSpeed;
	AIController->CustomPawn->ApplyModifiers( Modifiers );
}

void UZeroEnemyAISubstate::OnSubstateExited_Implementation()
{
}
