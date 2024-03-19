#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MXHCheatFunction.generated.h"

UENUM( BlueprintType )
enum class EMXHCheatFunctionType : uint8
{
	Action,
	Float,
	Int,
};

USTRUCT( BlueprintType )
struct FMXHCheatFunctionActionParams
{
	GENERATED_BODY()

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	bool IsToggled;

	/*UPROPERTY( EditAnywhere, BlueprintReadWrite )
	float FloatValue;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	int IntValue;*/
};

class UMXHCheatManager;

//  TODO: Add Category meta to UPROPERTYs

/**
 * 
 */
UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UMXHCheatFunction : public UObject
{
	GENERATED_BODY()
	
public:
	void Init( UMXHCheatManager* CheatManager );
	UFUNCTION( BlueprintCallable )
	void Cheat( FMXHCheatFunctionActionParams Params );

	UFUNCTION( BlueprintNativeEvent )
	void OnInit();
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category="Cheat Function" )
	void OnCheat();

	UFUNCTION( BlueprintCallable, BlueprintPure )
	virtual EMXHCheatFunctionType GetFunctionType() const { return EMXHCheatFunctionType::Action; }

	UWorld* GetWorld() const override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FText Name;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FText Category;
	/*UPROPERTY( EditAnywhere, BlueprintReadWrite )
	EMXHCheatFunctionType Type = EMXHCheatFunctionType::Action;*/
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	bool IsTogglable = false;

	UPROPERTY( BlueprintReadOnly )
	bool IsToggled = false;

	UPROPERTY( BlueprintReadOnly )
	UMXHCheatManager* CheatManager;
};

UCLASS( Abstract, Blueprintable )
class METROXHUNTER_API UMXHCheatFloatFunction : public UMXHCheatFunction
{
	GENERATED_BODY()

public:
	EMXHCheatFunctionType GetFunctionType() const override { return EMXHCheatFunctionType::Float; }

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	float MinValue = 0.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	float MaxValue = 1.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	bool ShouldSnap = true;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta=( EditCondition="ShouldSnap == true" ))
	float SnapStep = 0.1f;

	UPROPERTY( EditAnywhere, BlueprintReadWrite )
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