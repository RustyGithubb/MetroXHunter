#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MXHCheatFunction.generated.h"

class UMXHCheatManager;

/**
 * 
 */
UCLASS( Abstract, Blueprintable, BlueprintType )
class METROXHUNTER_API UMXHCheatFunction : public UObject
{
	GENERATED_BODY()
	
public:
	void Init( UMXHCheatManager* CheatManager );

	UFUNCTION( BlueprintNativeEvent )
	void OnInit();
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category="Cheat Function" )
	void OnCheat();

	UWorld* GetWorld() const override;

public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FText Name;
	UPROPERTY( EditAnywhere, BlueprintReadWrite )
	FText Category;

	UPROPERTY( BlueprintReadOnly )
	UMXHCheatManager* CheatManager;
};
