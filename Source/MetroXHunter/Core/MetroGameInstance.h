/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "MetroGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogMetroSaveSystem, Log, All );

class UMetroSaveGame;

/*
 * Custom Game Instance with Save & Load implementation.
 * TODO: Rename as UMetroGameInstance
 */
UCLASS()
class METROXHUNTER_API UMetroGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	void Init() override;

	UFUNCTION( BlueprintCallable, Category = "System|SaveLoad" )
	void CreateNewSave();
	void DeleteOldSaves();
	UFUNCTION( BlueprintCallable, Category = "System|SaveLoad" )
	void LoadLastSave();

	UFUNCTION( BlueprintCallable, Category = "System|SaveLoad" )
	void SaveLevel();
	UFUNCTION( BlueprintCallable, Category = "System|SaveLoad" )
	void LoadLevel();

	UFUNCTION( BlueprintCallable, Category = "GameInstance|LevelStreaming" )
	void RegisterStreamedLevel( const TSoftObjectPtr<UWorld> Level );
	UFUNCTION( BlueprintCallable, Category = "GameInstance|LevelStreaming" )
	void UnRegisterStreamedLevel( const TSoftObjectPtr<UWorld> Level );

	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable )
	void LoadLevelStreaming( const FName LevelName );
	UFUNCTION( BlueprintCallable )
	void LoadLevelStreamingByObject( const TSoftObjectPtr<UWorld> Level );

	UFUNCTION( BlueprintPure, Category = "System|SaveLoad")
	bool HasSaveGame() const;

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSaveLevel );
	UPROPERTY( BlueprintAssignable, Category = "System|SaveLoad" )
	FOnSaveLevel OnSaveLevel {};
	DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnLoadLevel );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "System|SaveLoad" )
	FOnLoadLevel OnLoadLevel {};

	/*
	 * Called when a level streaming has been entirely loaded.
	 * NOTE: This is supposed to be called by UMetroGameInstance::LoadLevelStreaming.
	 */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLevelStreamingLoad, FName, LevelName );
	UPROPERTY( BlueprintCallable, BlueprintAssignable, Category = "System|SaveLoad" )
	FOnLevelStreamingLoad OnLevelStreamingLoad {};

public:
	// TODO: Make it private and pass it inside delegates instead
	UPROPERTY( BlueprintReadOnly, Category = "System|SaveLoad" )
	UMetroSaveGame* SaveGame = nullptr;

private:
	const int32 UserIndex = 0;

	TSet<FName> CurrentStreamedLevelNames {};
};
