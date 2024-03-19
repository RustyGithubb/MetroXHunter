#include "MXHCheatFunction.h"

#include "MXHCheatManager.h"

void UMXHCheatFunction::Init( UMXHCheatManager* _CheatManager )
{
	CheatManager = _CheatManager;

	OnInit();
}

void UMXHCheatFunction::OnInit_Implementation() {}

void UMXHCheatFunction::OnCheat_Implementation() {}

UWorld* UMXHCheatFunction::GetWorld() const
{
	if ( !IsValid( CheatManager ) ) return nullptr;

	return CheatManager->GetWorld();
}
