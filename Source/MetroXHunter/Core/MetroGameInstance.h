/*
 * Implemented by Corentin Paya
 */

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AudioMixerDevice.h"
#include "AudioDevice.h"
#include "MetroGameInstance.generated.h"

DECLARE_LOG_CATEGORY_EXTERN( LogMetroSaveSystem, Log, All );

class UMetroSaveGame;
class UInventoryData;

USTRUCT( BlueprintType )
struct FAudioDeviceInfoBP
{
	GENERATED_BODY()

	/** Device Name */
	UPROPERTY( BlueprintReadOnly, Category = "Audio" )
	FString Name;

	/** Device ID */
	UPROPERTY( BlueprintReadOnly, Category = "Audio" )
	FString DeviceId;
};


/*
 * Custom Game Instance with Save & Load implementation.
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
	UFUNCTION( BlueprintImplementableEvent, BlueprintCallable )
	void UnloadLevelStreaming( const FName LevelName );


	UFUNCTION( BlueprintCallable )
	void LoadLevelStreamingByObject( const TSoftObjectPtr<UWorld> Level );

	UFUNCTION( BlueprintPure, Category = "System|SaveLoad")
	bool HasSaveGame() const;

	UFUNCTION( BlueprintCallable, Category = "System|Difficulty" )
	void SetAmmoMultiplier(float Multiplier);

	// Audio devices

	/* Get all audio devices available */
	UFUNCTION( BlueprintCallable, Category = "Audio" )
	TArray<FAudioDeviceInfoBP> GetAvailableOutputDevices() const;

	/* Change the audio devices based on the DeviceId
	 * @param DeviceId   Device ID choose
	 * @param bSuccess   True if the device change is a success
	 */
	UFUNCTION( BlueprintCallable, Category = "Audio" )
	void SetOutputDevice( const FString& DeviceId, bool& bSuccess );

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

	UPROPERTY( BlueprintReadOnly, Category = "System|SaveLoad" )
	UMetroSaveGame* TemporarySaveGame = nullptr;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Inventory|Data" )
	UInventoryData* InventoryDataAsset = nullptr;

	UPROPERTY( BlueprintReadOnly, Category = "System|Difficulty" )
	float AmmoMultiplier = 1.0f;

private:
	const int32 UserIndex = 0;

	TSet<FName> CurrentStreamedLevelNames {};
	TSet<FName> CurrentUnloadStreamedLevelNames {};
};
