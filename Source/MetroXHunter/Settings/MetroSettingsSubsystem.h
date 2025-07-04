/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "MetroSettingsSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogMetroSettings, Log, All );

class UGameLevelsData;
class UMetroSettingsSubsystem;

/*
 * Represent all categories available in the settings menu.
 * Only one category can be assigned to a SettingUnit at the same time.
 */
UENUM( BlueprintType )
enum class EMetroSettingCategory : uint8
{
	Gameplay,
	Display,
	Controls,
	Audio,
	LanguageAndSubtitles,
};

UENUM( BlueprintType )
enum class EMetroSettingType : uint8
{
	Undefined,
	Float,
	Bool,
	Enum,
};

UCLASS( Abstract, Blueprintable )
class UMetroSettingUnitBase : public UObject
{
	GENERATED_BODY()

	friend UMetroSettingsSubsystem;

public:
	/*
	 * 
	 */
	UFUNCTION( BlueprintCallable, Category = "MetroSettings" )
	void ApplySetting();
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "MetroSettings" )
	void ResetToDefaultValue();

	UFUNCTION( BlueprintPure, Category = "MetroSettings" )
	virtual EMetroSettingType GetSettingType() const;

	UWorld* GetWorld() const override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	FText DisplayName {};
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	EMetroSettingCategory Category = EMetroSettingCategory::Gameplay;
	/*
	 * Whenever the setting should be re-applied when a game map starts.
	 * Useful for gameplay-related settings that need a game context to function.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	bool bShouldReapplyUponGameStart = false;

	UPROPERTY( BlueprintReadOnly, Category = "MetroSettings" )
	UMetroSettingsSubsystem* Subsystem = nullptr;

protected:
	void ApplySetting( bool bInGame );

	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "MetroSettings" )
	void OnInitialize();
	/*
	 * Implementation of the setting.
	 * @param bInGame Whenever the current level is not the main menu.
	 */
	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "MetroSettings" )
	void OnApplySetting( bool bInGame );
};

UCLASS( Abstract, Blueprintable )
class UMetroSettingUnitFloat : public UMetroSettingUnitBase
{
	GENERATED_BODY()

public:
	/*
	 * Set a new value and apply this setting.
	 */
	UFUNCTION( BlueprintCallable, Category = "MetroSettings" )
	void SetValue( float NewValue );

	void ResetToDefaultValue_Implementation() override;

	virtual EMetroSettingType GetSettingType() const override;

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	float Value = 0.0f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "MetroSettings" )
	float DefaultValue = 0.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "MetroSettings" )
	FFloatRange ValueRange { 0.0f, 100.0f };
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "MetroSettings" )
	float StepSize = 0.01f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "MetroSettings" )
	bool bShowValueAsPercent = false;
};

UCLASS( Abstract, Blueprintable )
class UMetroSettingUnitBool : public UMetroSettingUnitBase
{
	GENERATED_BODY()

public:
	/*
	* Set a new value and apply this setting.
	*/
	UFUNCTION( BlueprintCallable, Category = "MetroSettings" )
	void SetValue( bool bNewValue );

	void ResetToDefaultValue_Implementation() override;

	virtual EMetroSettingType GetSettingType() const override;

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	bool bValue = false;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "MetroSettings" )
	bool bDefaultValue = false;
};

UCLASS( Abstract, Blueprintable )
class UMetroSettingUnitEnum : public UMetroSettingUnitBase
{
	GENERATED_BODY()

public:
	/*
	 * Set a new value and apply this setting.
	 * 
	 * The value is wrap on the number of elements inside EnumTexts.
	 */
	UFUNCTION( BlueprintCallable, Category = "MetroSettings" )
	void SetValue( int32 NewIndexValue );

	void ResetToDefaultValue_Implementation() override;

	virtual EMetroSettingType GetSettingType() const override;

public:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	int32 IndexValue = 0;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "MetroSettings" )
	int32 DefaultIndexValue = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "MetroSettings" )
	TArray<FText> EnumTexts {};
};

/**
 * 
 */
UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UMetroSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void Initialize( FSubsystemCollectionBase& Collection ) override;

	void ApplySettingsUponGameStart();

	UFUNCTION( BlueprintCallable, Category = "MetroSettings", meta = ( DeterminesOutputType = "Class", ReturnDisplayName = "SettingUnit" ))
	UMetroSettingUnitBase* FindSettingOfClass( const TSubclassOf<UMetroSettingUnitBase> Class );

	UFUNCTION( BlueprintPure, Category = "MetroSettings" )
	bool IsCurrentlyInGame() const;

	UFUNCTION( BlueprintPure, Category = "MetroSettings" )
	const TArray<UMetroSettingUnitBase*>& GetSettingUnits() const;

public:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	UGameLevelsData* GameLevelsData = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	FName SettingUnitsDirectoryPath = TEXT( "/Game/Design/Settings/Units/" );

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "MetroSettings" )
	TArray<TSubclassOf<UMetroSettingUnitBase>> SettingClassOrders {};

private:
	void InstantiateSettingUnit( TSubclassOf<UMetroSettingUnitBase> Class );

private:
	UPROPERTY()
	TArray<UMetroSettingUnitBase*> SettingUnits {};
};
