/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BaseSavedData.h"

#include "InstancedStruct.h"

#include "SaveLoadComponent.generated.h"

class UMetroGameInstance;

/*
 * Component that saves and loads all the specified components of the owner.
 * The GUID is also stored in that component.
 */
UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API USaveLoadComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USaveLoadComponent();

	virtual void BeginPlay() override;
	virtual void PostInitProperties() override;

	UFUNCTION( BlueprintCallable )
	void GenerateActorID();
	const FGuid& GetActorID() const;

	UFUNCTION( BlueprintCallable )
	void Save();
	UFUNCTION(BlueprintCallable)
	void Load();

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSaveActor );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "System|SaveLoad" )
	FOnSaveActor OnSaveActor {};
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnLoadActor );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "System|SaveLoad" )
	FOnLoadActor OnLoadActor {};

public:
	/* All the components that needs to be saved for this actor */
	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	TArray<UActorComponent*> ComponentsToSave {};

	UPROPERTY( EditAnywhere, BlueprintReadOnly )
	FGuid DefaultGUID {};
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, DuplicateTransient )
	FGuid ActorGUID {};

	/* Extra data specific to this actor class that needs to be saved */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( BaseStruct = "/Script/MetroXHunter.BaseSavedData" ) )
	FInstancedStruct SavedData {};

	UPROPERTY(BlueprintReadOnly)
	bool bIsDataLoaded = false;

private:
	UMetroGameInstance* GameInstance = nullptr;
};
