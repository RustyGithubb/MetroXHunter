/*
 * Implemented by BARRAU Benoit
 */

#include "DialogueManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentDialogueIndex = 0;
	CurrentAudioComponent = nullptr;
	bDialoguePaused = false;
	LastInterruptedSpeaker = EDialogueSpeaker::Unknown;
	CustomAudioComponent = nullptr;
}

void UDialogueManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDialogueManagerComponent::StartDialogue()
{
	if ( !Dialogue )
	{
		UE_LOG( LogTemp, Warning, TEXT( "The dialogue DataTable is not assigned!" ) );
		return;
	}

	// Reset dialogue index and interruption state.
	CurrentDialogueIndex = 0;
	bDialoguePaused = false;
	LastDialogueIndexMap.Empty();
	LastInterruptedSpeaker = EDialogueSpeaker::Unknown;

	// If random namespace selection is enabled, filter dialogue rows based on the set of specified namespaces.
	if ( bUseRandomNamespaceSelection )
	{
		// Clear previous rows.
		DialogueRows.Empty();

		static const FString ContextString( TEXT( "DialogueContext" ) );
		TArray<FName> RowNames = Dialogue->GetRowNames();

		// Loop through all rows and add those that match any of the random dialogue namespaces.
		for ( const FName& RowName : RowNames )
		{
			FDialogue* Row = Dialogue->FindRow<FDialogue>( RowName, ContextString );
			if ( Row && RandomDialogueNamespaces.Contains( Row->DialogueNamespace ) )
			{
				DialogueRows.Add( Row );
			}
		}

		// If no dialogue rows were found, log a warning and exit.
		if ( DialogueRows.Num() == 0 )
		{
			UE_LOG( LogTemp, Warning, TEXT( "No dialogue found for the random namespaces." ) );
			return;
		}

		// Select one dialogue row randomly from the filtered list.
		int32 RandomRowIndex = FMath::RandRange( 0, DialogueRows.Num() - 1 );
		FDialogue* SelectedRow = DialogueRows[RandomRowIndex];

		// Keep only the randomly selected dialogue row.
		DialogueRows.Empty();
		DialogueRows.Add( SelectedRow );
	}
	else
	{
		// Otherwise, load dialogue rows normally filtered by the DialogueNamespace property.
		LoadDialogueRows();
	}

	if ( DialogueRows.Num() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "No dialogue found for DialogueNamespace: %s" ), *DialogueNamespace.ToString() );
		return;
	}

	// Start playing from the first dialogue row
	// (which will be the only one in the random selection case).
	PlayDialogueRow( CurrentDialogueIndex );
}


void UDialogueManagerComponent::LoadDialogueRows()
{
	DialogueRows.Empty();

	static const FString ContextString( TEXT( "DialogueContext" ) );
	TArray<FName> RowNames = Dialogue->GetRowNames();

	for ( const FName& RowName : RowNames )
	{
		FDialogue* Row = Dialogue->FindRow<FDialogue>( RowName, ContextString );
		// Filter based on DialogueNamespace.
		if ( Row && Row->DialogueNamespace == DialogueNamespace )
		{
			DialogueRows.Add( Row );
		}
	}

	// Removed sorting logic so the rows remain in the original order.
}

void UDialogueManagerComponent::PlayDialogueRow( int32 RowIndex )
{
	// Broadcast the event indicating that the dialogue has started.
	if ( OnDialogueStarted.IsBound() )
	{
		OnDialogueStarted.Broadcast();
	}

	if ( !DialogueRows.IsValidIndex( RowIndex ) )
	{
		ClearDialogueText();
		return;
	}

	FDialogue* CurrentRow = DialogueRows[RowIndex];
	if ( !CurrentRow )
	{
		OnSoundFinished();
		return;
	}

	// Construct the displayed text: "Speaker: Text".
	FString SpeakerName = GetSpeakerName( CurrentRow->Speaker );
	FString DisplayText = FString::Printf( TEXT( "%s: %s" ), *SpeakerName, *CurrentRow->Text.ToString() );
	UpdateDialogueText( DisplayText );

	// Play the sound associated with the dialogue line.
	if ( CurrentRow->Sound )
	{
		// If a custom AudioComponent is set, use it.
		if ( CustomAudioComponent )
		{
			CurrentAudioComponent = CustomAudioComponent;
			// Stop the current sound if it is playing.
			if ( CurrentAudioComponent->IsPlaying() )
			{
				CurrentAudioComponent->Stop();
			}
			// Set the new sound and bind the finish event.
			CurrentAudioComponent->SetSound( CurrentRow->Sound );
			CurrentAudioComponent->OnAudioFinished.Clear();
			CurrentAudioComponent->OnAudioFinished.AddDynamic( this, &UDialogueManagerComponent::OnSoundFinished );
			CurrentAudioComponent->Play();
		}
		else
		{
			// Otherwise, spawn an AudioComponent using SpawnSound2D.
			CurrentAudioComponent = UGameplayStatics::SpawnSound2D( this, CurrentRow->Sound );
			if ( CurrentAudioComponent )
			{
				CurrentAudioComponent->OnAudioFinished.AddDynamic( this, &UDialogueManagerComponent::OnSoundFinished );
			}
			else
			{
				OnSoundFinished();
			}
		}
	}
	else
	{
		// If no sound is defined, move directly to the next dialogue line.
		OnSoundFinished();
	}
}

void UDialogueManagerComponent::OnSoundFinished()
{
	// Stop and clean up the AudioComponent.
	if ( CurrentAudioComponent )
	{
		CurrentAudioComponent->Stop();
		// Do not reset the custom AudioComponent if it is used.
		if ( CurrentAudioComponent != CustomAudioComponent )
		{
			CurrentAudioComponent = nullptr;
		}
	}

	// Advance to the next dialogue line.
	CurrentDialogueIndex++;
	if ( DialogueRows.IsValidIndex( CurrentDialogueIndex ) )
	{
		PlayDialogueRow( CurrentDialogueIndex );
	}
	else
	{
		// End of dialogue: clear the UI.
		ClearDialogueText();
	}
}

void UDialogueManagerComponent::UpdateDialogueText( const FString& NewText )
{
	// Broadcast the new dialogue text to the UI.
	if ( OnDialogueTextUpdated.IsBound() )
	{
		OnDialogueTextUpdated.Broadcast( FText::FromString( NewText ) );
	}
	else
	{
		UE_LOG( LogTemp, Warning, TEXT( "No UI bound to receive dialogue updates." ) );
	}
}

void UDialogueManagerComponent::ClearDialogueText()
{
	UpdateDialogueText( TEXT( "" ) );
}

FString UDialogueManagerComponent::GetSpeakerName( EDialogueSpeaker Speaker ) const
{
	switch ( Speaker )
	{
	case EDialogueSpeaker::Charles:
		return FString( "Charles" );
	case EDialogueSpeaker::Iris:
		return FString( "Iris" );
	case EDialogueSpeaker::Unknown:
		return FString( "?????" );
	default:
		return FString( "None" );
	}
}

void UDialogueManagerComponent::SetDialogueNamespace( FName NewNamespace )
{
	DialogueNamespace = NewNamespace;
}

FName UDialogueManagerComponent::GetDialogueNamespace() const
{
	return DialogueNamespace;
}

void UDialogueManagerComponent::SetCustomAudioComponent( UAudioComponent* NewAudioComponent )
{
	CustomAudioComponent = NewAudioComponent;
}

void UDialogueManagerComponent::InterruptDialogueDueToDamage()
{
	// Stop the current audio if playing.
	if ( CurrentAudioComponent )
	{
		CurrentAudioComponent->Stop();
		CurrentAudioComponent = nullptr;
	}

	// Record the current dialogue line for the speaker.
	if ( DialogueRows.IsValidIndex( CurrentDialogueIndex ) )
	{
		FDialogue* CurrentRow = DialogueRows[CurrentDialogueIndex];
		if ( CurrentRow )
		{
			LastDialogueIndexMap.FindOrAdd( CurrentRow->Speaker ) = CurrentDialogueIndex;
			LastInterruptedSpeaker = CurrentRow->Speaker;
		}
	}

	bDialoguePaused = true;

	// After a delay (e.g., 3 seconds), resume the dialogue.
	if ( GetWorld() )
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer( TimerHandle, this, &UDialogueManagerComponent::ResumeDialogueFromInterruption, 3.0f, false );
	}
}

void UDialogueManagerComponent::ResumeDialogueFromInterruption()
{
	if ( !bDialoguePaused )
	{
		return;
	}

	bDialoguePaused = false;

	// If a speaker was interrupted, resume from that speaker's last dialogue line.
	if ( LastInterruptedSpeaker != EDialogueSpeaker::None )
	{
		int32* pIndex = LastDialogueIndexMap.Find( LastInterruptedSpeaker );
		if ( pIndex && DialogueRows.IsValidIndex( *pIndex ) )
		{
			CurrentDialogueIndex = *pIndex;
			// Restart the dialogue line from the beginning.
			PlayDialogueRow( CurrentDialogueIndex );
		}
	}
}

