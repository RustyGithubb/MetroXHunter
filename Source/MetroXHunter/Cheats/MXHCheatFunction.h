#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MXHCheatFunction.generated.h"

/**
 * Enum representing the type of a CheatFunction, preventing multiple casts to find
 * its underlying class.
 */
UENUM( BlueprintType )
enum class EMXHCheatFunctionType : uint8
{
	Action,
	Float,
	Int,
	Selection,
};

/**
 * Structure representing the parameters to pass inside the 'Cheat' method.
 * Currently, it doesn't do much by itself, its existence only justify for maintenability and
 * feature scaling reasons.
 */
USTRUCT( BlueprintType )
struct FMXHCheatFunctionActionParams
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	bool bIsToggled;
};

class UMXHCheatManager;

/**
 * Base class for creating cheat commands, intended to be used in the CheatMenu.
 *
 * It is designed to be fully implementable by Blueprints located inside the folder
 * specified by the game's CheatManager.
 *
 * Docs: https://www.notion.so/metro-hunter/Cheat-Function-28fa417fea184d3dbd116b2f9b493367
 */
UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UMXHCheatFunction : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Initializes the cheat function, eventually calling its blueprint native event 'OnInit'.
	 *
	 * @param CheatManager	Cheat Manager to use, MUST be valid
	 */
	void Init( UMXHCheatManager* CheatManager );

	/**
	 * Triggers cheat behaviour, eventually calling its blueprint native event 'OnCheat'.
	 *
	 * @param Params		Parameters to apply before the cheat triggers
	 */
	UFUNCTION( BlueprintCallable, Category = "Cheat Function" )
	void Cheat( FMXHCheatFunctionActionParams Params );
	/**
	 * Triggers cheat behaviour with default parameters, eventually calling its blueprint native
	 * event 'OnCheat'.
	 */
	void Cheat();

	/**
	 * Event called after the cheat has been initialized by the CheatManager.
	 * Designed to be called only once.
	 */
	UFUNCTION( BlueprintNativeEvent )
	void OnInit();
	/**
	 * Event called when the cheat is supposed to do something.
	 *
	 * Typically, this is called when the player provides an input inside the CheatMenu,
	 * whether it's a value change or a button press.
	 *
	 * This is also called after the initialization if 'bShouldStartToggled' is set to true.
	 */
	UFUNCTION( BlueprintNativeEvent )
	void OnCheat();

	/**
	 * Returns the cheat function type. Used to assume the class type of a cheat by avoiding casts.
	 *
	 * @return				Cheat function type
	 */
	UFUNCTION( BlueprintCallable, BlueprintPure, Category = "Cheat Function" )
	virtual EMXHCheatFunctionType GetFunctionType() const { return EMXHCheatFunctionType::Action; }

	UWorld* GetWorld() const override;

public:
	/**
	 * Displayed name of the cheat
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function" )
	FText Name;
	/**
	 * Category name to sort the cheats inside the CheatMenu, there is a list of categories with
	 * their own order priority inside the CheatManager, you MUST match their names.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function" )
	FText Category;
	/**
	 * Give identical keys to different CheatFunctions to pair them together visually in the CheatMenu.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function" )
	FText PairingKey;
	/**
	 * Set whether the cheat has a ON/OFF state. The toggle state will be switched when the player
	 * press the button.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function" )
	bool bIsTogglable = false;
	/**
	 * If 'bIsToggable' is set to true, it states whether the cheat should be called as toggled
	 * after its initialization.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function", meta = ( EditCondition = "bIsTogglable" ) )
	bool bShouldStartToggled = false;

	/**
	 * If 'bIsToggable' is set to true, it represents the toggle state.
	 */
	UPROPERTY( BlueprintReadOnly, Category = "Cheat Function" )
	bool bIsToggled = false;

	/**
	 * The CheatManager that owns this cheat.
	 */
	UPROPERTY( BlueprintReadOnly, Category = "Cheat Function" )
	UMXHCheatManager* CheatManager;
};

/*
 * Cheat Function subclass focused on providing a float value editable through a slider
 * by the player.
 */
UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UMXHCheatFloatFunction : public UMXHCheatFunction
{
	GENERATED_BODY()

public:
	EMXHCheatFunctionType GetFunctionType() const override { return EMXHCheatFunctionType::Float; }

public:
	/**
	 * Maximum and minimum bounds used to constrain the value.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function|Float Parameter" )
	FFloatRange ValueRange { 0.0f, 1.0f };
	/**
	 * Whether the value should snap to the specified 'SnapStep' when edited through the slider.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function|Float Parameter" )
	bool bShouldSnap = true;
	/**
	 * If 'bShouldSnap' is set to true, the increment with which the value is snapped to.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function|Float Parameter", meta = ( EditCondition = "bShouldSnap" ) )
	float SnapStep = 0.1f;

	/**
	 * The current value selected by the player.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function|Float Parameter" )
	float Value = 0.0f;
};

UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UMXHCheatIntFunction : public UMXHCheatFunction
{
	GENERATED_BODY()

public:
	EMXHCheatFunctionType GetFunctionType() const override { return EMXHCheatFunctionType::Int; }

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int MinValue = 0;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int MaxValue = 1;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int Value = 0;
};

/*
 * Cheat Function subclass focused on providing an index through an array of possibilities
 * to the player.
 */
UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UMXHCheatSelectionFunction : public UMXHCheatFunction
{
	GENERATED_BODY()

public:
	/**
	 * Moves the index to the right (i.e. towards the last element) and wrap it to the first element
	 * if it goes above the last index. Calls the cheat event afterward.
	 */
	UFUNCTION( BlueprintCallable, Category = "Cheat Function" )
	void CycleToNext();
	/**
	 * Moves the index to the left (i.e. towards the first element) and wrap it to the last element
	 * if it goes below zero. Calls the cheat event afterward.
	 */
	UFUNCTION( BlueprintCallable, Category = "Cheat Function" )
	void CycleToPrevious();

	EMXHCheatFunctionType GetFunctionType() const override { return EMXHCheatFunctionType::Selection; }

public:
	/**
	 * Array of strings representing the displayed names of all possible values. This array is used
	 * to wrap the index around.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function|Selection Parameter" )
	TArray<FString> ValueNames;
	/**
	 * Index of the currently selected value inside the 'ValueNames' array. Depending on what you're
	 * doing, you may want to create an other array representing the actual data values which you
	 * index with this value.
	 */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Cheat Function|Selection Parameter" )
	int Index = 0;
};