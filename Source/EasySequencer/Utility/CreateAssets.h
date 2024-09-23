/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CreateAssets.generated.h"

class UActorComponent;

 /**
  *
  */
UCLASS()
class EASYSEQUENCER_API UCreateAssets : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, Category = "EasySequencer|Component" )
	static UActorComponent* AddComponent( 
		TSubclassOf<UActorComponent> ComponentClass,
		AActor* Actor,
		USceneComponent* ParentComponent,
		FName Name 
	);

	UFUNCTION( BlueprintCallable, Category = "EasySequencer|Component" )
	static void RemoveComponent( UActorComponent* ActorComponent );

	UFUNCTION( BlueprintCallable, Category = "EasySequencer|Debug" )
	static void ShowFormattedDialog( const FString& InFileName );

	UFUNCTION( BlueprintCallable, Category = "EasySequencer|SaveLoad" )
	static void UpdateActor( AActor* Actor );

	UFUNCTION( BlueprintCallable, Category = "EasySequencer|SaveLoad" )
	static void SavePackage( UObject* Object );
};
