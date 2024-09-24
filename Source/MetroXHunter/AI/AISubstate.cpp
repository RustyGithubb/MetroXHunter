/* 
 * Implemented by Arthur Cathelain (arkaht) 
 */

#include "AI/AISubstate.h"
#include "AI/AISubstateManagerComponent.h"

void UAISubstate::OnSubstateInitialized_Implementation() {}

void UAISubstate::OnSubstateEntered_Implementation() {}

void UAISubstate::OnSubstateExited_Implementation() {}

EAISubstateSwitchOrder UAISubstate::TickSubstate_Implementation( float DeltaTime ) 
{
	return EAISubstateSwitchOrder::Stay;
}

UWorld* UAISubstate::GetWorld() const
{
	if ( Manager == nullptr ) return nullptr;
	return Manager->GetWorld();
}
