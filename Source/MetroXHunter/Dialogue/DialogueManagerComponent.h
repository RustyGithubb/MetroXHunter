#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "DialogueManagerComponent.generated.h"

/**
 * Enumeration representing the dialogue speakers.
 * The possible speakers are Charles and Iris.
 */
UENUM( BlueprintType )
enum class EDialogueSpeaker : uint8
{
	Charles UMETA( DisplayName = "Charles" ),
	Iris    UMETA( DisplayName = "Iris" ),
	Inconnu    UMETA( DisplayName = "?????" )
};

/**
 * Structure representing a dialogue line in the DataTable.
 * This structure must match the columns defined in the editor.
 */
USTRUCT( BlueprintType )
struct FDialogue : public FTableRowBase
{
	GENERATED_BODY()

public:
	// Used to filter the dialogue lines.
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dialogue" )
	FName DialogueNamespace;

	// Speaker as an enumeration.
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dialogue" )
	EDialogueSpeaker Speaker;

	// Text to display (localizable).
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dialogue" )
	FText Text;

	// Sound to play during the dialogue.
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dialogue" )
	USoundBase* Sound;
};

// Delegate declaration to update dialogue text.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnDialogueTextUpdated, const FText&, NewDialogueText );

UCLASS( Blueprintable, ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class METROXHUNTER_API UDialogueManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueManagerComponent();

	virtual void BeginPlay() override;

public:
	/**
	 * Starts playing the dialogue.
	 * This function filters the DataTable (named here "Dialogue") based on the chosen DialogueNamespace
	 * and sequentially plays each line (text and sound).
	 *
	 * It will be called by the BPAC_DialogueTriggerReceiver_Component.
	 */
	UFUNCTION( BlueprintCallable, Category = "Dialogue" )
	void StartDialogue();

	// Getter and setter for the DialogueNamespace property.
	UFUNCTION( BlueprintCallable, Category = "Dialogue" )
	void SetDialogueNamespace( FName NewNamespace );

	UFUNCTION( BlueprintCallable, Category = "Dialogue" )
	FName GetDialogueNamespace() const;

	// Delegate event to update the UI with the current dialogue text.
	UPROPERTY( BlueprintAssignable, Category = "Dialogue" )
	FOnDialogueTextUpdated OnDialogueTextUpdated;

public:
	// The DialogueNamespace used to filter dialogue lines (modifiable in the editor).
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dialogue" )
	FName DialogueNamespace;

	// Reference to the existing DataTable in your project.
	// Assign this in the editor; this DataTable should be named "Dialogue".
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Dialogue" )
	UDataTable* Dialogue;

private:
	// Loads and filters the dialogue lines from the DataTable based on the chosen DialogueNamespace.
	void LoadDialogueRows();

	// Plays the dialogue line at the specified index.
	void PlayDialogueRow( int32 RowIndex );

	// Callback triggered when the sound playback finishes.
	UFUNCTION()
	void OnSoundFinished();

	// Updates the UI by broadcasting the dialogue text.
	void UpdateDialogueText( const FString& NewText );

	// Clears the UI dialogue text (broadcasts an empty text).
	void ClearDialogueText();

	// Helper function to convert the Speaker enumeration to a string.
	FString GetSpeakerName( EDialogueSpeaker Speaker ) const;

private:
	// Array containing the filtered dialogue lines from the DataTable.
	TArray<FDialogue*> DialogueRows;

	// Index of the currently played dialogue line.
	int32 CurrentDialogueIndex;

	// Audio component used to play the sound of the current dialogue line.
	UPROPERTY()
	UAudioComponent* CurrentAudioComponent;
};
