/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#include "Settings/MetroSettingsSubsystem.h"

#include "GameLevelsData.h"

#include "Library/UtilityLibrary.h"

DEFINE_LOG_CATEGORY( LogMetroSettings );

void UMetroSettingUnitBase::ApplySetting()
{
	bool bInGame = Subsystem->IsCurrentlyInGame();
	ApplySetting( bInGame );
}

void UMetroSettingUnitBase::ResetToDefaultValue_Implementation()
{
	UE_LOG(
		LogMetroSettings, Warning,
		TEXT( "MetroSettingUnit '%s' didn't implement ResetToDefaultValue." ),
		*GetName()
	);
}

EMetroSettingType UMetroSettingUnitBase::GetSettingType() const
{
	return EMetroSettingType::Undefined;
}

UWorld* UMetroSettingUnitBase::GetWorld() const
{
	if ( Subsystem == nullptr ) return nullptr;
	return Subsystem->GetWorld();
}

void UMetroSettingUnitBase::ApplySetting( bool bInGame )
{
	OnApplySetting( bInGame );

	UE_LOG( LogMetroSettings, Log, TEXT( "MetroSettingUnit '%s' has been applied." ), *GetName() );
}

void UMetroSettingUnitBase::OnInitialize_Implementation()
{
}

void UMetroSettingUnitBase::OnApplySetting_Implementation( bool bInGame )
{
	UE_LOG(
		LogMetroSettings, Warning,
		TEXT( "MetroSettingUnit '%s' didn't implement ApplySetting." ),
		*GetName()
	);
}


void UMetroSettingUnitFloat::SetValue( float NewValue )
{
	Value = FMath::Clamp( NewValue, ValueRange.GetLowerBoundValue(), ValueRange.GetUpperBoundValue() );

	ApplySetting();
}

void UMetroSettingUnitFloat::ResetToDefaultValue_Implementation()
{
	SetValue( DefaultValue );
}

EMetroSettingType UMetroSettingUnitFloat::GetSettingType() const
{
	return EMetroSettingType::Float;
}


void UMetroSettingUnitBool::SetValue( bool bNewValue )
{
	bValue = bNewValue;

	ApplySetting();
}

void UMetroSettingUnitBool::ResetToDefaultValue_Implementation()
{
	SetValue( bDefaultValue );
}

EMetroSettingType UMetroSettingUnitBool::GetSettingType() const
{
	return EMetroSettingType::Bool;
}


void UMetroSettingUnitEnum::SetValue( int32 NewIndexValue )
{
	IndexValue = NewIndexValue % EnumTexts.Num();

	ApplySetting();
}

void UMetroSettingUnitEnum::ResetToDefaultValue_Implementation()
{
	SetValue( DefaultIndexValue );
}

EMetroSettingType UMetroSettingUnitEnum::GetSettingType() const
{
	return EMetroSettingType::Enum;
}


void UMetroSettingsSubsystem::Initialize( FSubsystemCollectionBase& Collection )
{
	Super::Initialize( Collection );

	UUtilityLibrary::ForceLoadAssetsAtPath( SettingUnitsDirectoryPath );

	// Instantiate all our setting unit blueprints
	for ( TObjectIterator<UClass> It; It; ++It )
	{
		UClass* Class = *It;

		// Filter out non-subclasses
		if ( !Class->IsChildOf<UMetroSettingUnitBase>() ) continue;

		// Filter out the base class
		if ( Class->HasAnyClassFlags( CLASS_Abstract ) ) continue;

		// Filter out any non-blueprints classes
		if ( !Class->GetName().RemoveFromStart( "BP_" ) ) continue;

		// Instantiate setting unit
		InstantiateSettingUnit( Class );
	}

	UE_LOG( LogMetroSettings, Log, TEXT( "Total of %d Setting Units" ), SettingUnits.Num() );

	// Sort settings by their orders
	SettingUnits.Sort( 
		[&]( const UMetroSettingUnitBase& A, const UMetroSettingUnitBase& B )
		{
			const int32 OrderA = SettingClassOrders.Find( A.GetClass() );
			const int32 OrderB = SettingClassOrders.Find( B.GetClass() );

			if ( OrderA == INDEX_NONE || OrderB == INDEX_NONE )
			{
				return A.DisplayName.ToString() < B.DisplayName.ToString();
			}
			
			return OrderA < OrderB;
		} 
	);

	// Initialize all our setting units once they are all created
	// so they can access each other if needed.
	for ( UMetroSettingUnitBase* SettingUnit : SettingUnits )
	{
		SettingUnit->OnInitialize();
	}

	// Apply all settings to their default value once they are all initialized.
	for ( UMetroSettingUnitBase* SettingUnit : SettingUnits )
	{
		UE_LOG( LogMetroSettings, Log, TEXT( "MetroSettingUnit '%s' has been applied." ), *SettingUnit->GetName() );
		SettingUnit->ResetToDefaultValue();
	}
}

void UMetroSettingsSubsystem::ApplySettingsUponGameStart()
{
	UE_LOG( LogMetroSettings, Log, TEXT( "Applying gameplay-related settings..." ) );

	bool bInGame = IsCurrentlyInGame();
	for ( UMetroSettingUnitBase* SettingUnit : SettingUnits )
	{
		if ( SettingUnit->bShouldReapplyUponGameStart )
		{
			SettingUnit->ApplySetting( bInGame );
		}
	}
}

UMetroSettingUnitBase* UMetroSettingsSubsystem::FindSettingOfClass( const TSubclassOf<UMetroSettingUnitBase> Class )
{
	for ( UMetroSettingUnitBase* SettingUnit : SettingUnits )
	{
		if ( SettingUnit->GetClass() == Class )
		{
			return SettingUnit;
		}
	}

	return nullptr;
}

bool UMetroSettingsSubsystem::IsCurrentlyInGame() const
{
	if ( GameLevelsData == nullptr )
	{
		UE_LOG( LogMetroSettings, Error, TEXT( "No reference to a GameLevelsData!" ) )
		return false;
	}

	const FName MenuLevelName = UUtilityLibrary::GetLevelName( GameLevelsData->MenuLevel.Level );
	const FName CurrentLevelName = UUtilityLibrary::GetLevelName( GetWorld() );

	UUtilityLibrary::LogMessage( TEXT( "MetroSettingsSubsystem: %s != %s\n" ), *MenuLevelName.ToString(), *CurrentLevelName.ToString() );

	return MenuLevelName != CurrentLevelName;
}

const TArray<UMetroSettingUnitBase*>& UMetroSettingsSubsystem::GetSettingUnits() const
{
	return SettingUnits;
}

void UMetroSettingsSubsystem::InstantiateSettingUnit( TSubclassOf<UMetroSettingUnitBase> Class )
{
	UMetroSettingUnitBase* SettingUnit = NewObject<UMetroSettingUnitBase>( this, Class );
	SettingUnit->Subsystem = this;
	SettingUnits.Add( SettingUnit );

	UE_LOG( LogMetroSettings, Log, TEXT( "New Setting Unit: %s" ), *Class->GetName() )

#ifdef WITH_EDITOR
	int32 FoundOccurences = 0;
	for ( const TSubclassOf<UMetroSettingUnitBase> SettingClass : SettingClassOrders )
	{
		if ( SettingClass == Class )
		{
			FoundOccurences++;
		}
	}
	if ( FoundOccurences == 0 )
	{
		UUtilityLibrary::PrintError(
			TEXT( "Settings: %s doesn't have a configured order, please set it in BP_MetroSettingsSubsystem!" ),
			*Class->GetName()
		);
	}
	if ( FoundOccurences > 1 )
	{
		UUtilityLibrary::PrintError(
			TEXT( "Settings: %s is present multiple times in the BP_MetroSettingsSubsystem!" ),
			*Class->GetName()
		);
	}
#endif
}
