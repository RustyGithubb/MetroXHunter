#include "Cheats/CheatFunction.h"

#include "MXHCheatManager.h"

void UCheatFunction::Init( UMXHCheatManager* _CheatManager )
{
	CheatManager = _CheatManager;

	OnInit();
}

void UCheatFunction::OnInit_Implementation() {}

void UCheatFunction::OnCheat_Implementation() {}

UWorld* UCheatFunction::GetWorld() const
{
	if ( !IsValid( CheatManager ) ) return nullptr;

	return CheatManager->GetWorld();
}
