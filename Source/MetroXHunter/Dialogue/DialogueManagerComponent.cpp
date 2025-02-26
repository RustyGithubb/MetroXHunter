/*
 * Implemented by BARRAU Benoit
 */

#include "DialogueManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentDialogueIndex = 0;
	CurrentAudioComponent = nullptr;
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

	// Reset the dialogue index.
	CurrentDialogueIndex = 0;

	// Load and filter the dialogue lines from the DataTable based on the chosen DialogueNamespace.
	LoadDialogueRows();

	if ( DialogueRows.Num() == 0 )
	{
		UE_LOG( LogTemp, Warning, TEXT( "No dialogue found for DialogueNamespace: %s" ), *DialogueNamespace.ToString() );
		return;
	}

	// Start playing from the first line.
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

	// Removed the sorting logic so the rows stay in the original order.
	// Maybe add a new sorting logic if necessary
}


void UDialogueManagerComponent::PlayDialogueRow( int32 RowIndex )
{
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
	else
	{
		// If no sound is defined, move on immediately.
		OnSoundFinished();
	}
}

void UDialogueManagerComponent::OnSoundFinished()
{
	// Clean up the audio component.
	if ( CurrentAudioComponent )
	{
		CurrentAudioComponent->Stop();
		CurrentAudioComponent = nullptr;
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
	// Instead of using debug text, broadcast the new dialogue text to the UI.
	if ( OnDialogueTextUpdated.IsBound() )
	{
		OnDialogueTextUpdated.Broadcast( FText::FromString( NewText ) );
	}
	else
	{
		// Optionally, you can log if no one is bound.
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
	default:
		return FString( "Unknown" );
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
