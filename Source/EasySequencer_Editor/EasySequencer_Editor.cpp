/*
 * Implemented by Corentin Paya
 */

#include "EasySequencer_Editor.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

#include "Editor/EditorEngine.h"
#include "EditorUtilityWidget.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilitySubsystem.h"

#define LOCTEXT_NAMESPACE "FEasySequencer_EditorModule"

void FEasySequencer_EditorModule::StartupModule()
{
	// Register a function to be called when menu system is initialized
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(
			this,
			&FEasySequencer_EditorModule::RegisterMenuExtensions
		)
	);
}

void FEasySequencer_EditorModule::ShutdownModule()
{
	// Unregister the startup function
	UToolMenus::UnRegisterStartupCallback( this );

	// Unregister all our menu extensions
	UToolMenus::UnregisterOwner( this );
}

void FEasySequencer_EditorModule::RegisterMenuExtensions()
{
	// Use the current object as the owner of the menus
	// This allows us to remove all our custom menus when the 
	// module is unloaded (see ShutdownModule below)
	FToolMenuOwnerScoped OwnerScoped( this );

	// Extend the "PlayToolBar" section of the main toolbar
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(
		"LevelEditor.LevelEditorToolBar.PlayToolBar" );
	FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection( "PlatformsMenu" );

	ToolbarSection.AddEntry(
		FToolMenuEntry::InitToolBarButton(
			TEXT( "MyCustomButtonName" ),
			FExecuteAction::CreateRaw( this, &FEasySequencer_EditorModule::OnButtonClicked ),
			INVTEXT( "ES Button" ),
			INVTEXT( "Easy Sequencer Tool" ),
			FSlateIcon( FAppStyle::GetAppStyleSetName(), "PlayWorld.PlayInCameraLocation" )
		)
	);
}

void FEasySequencer_EditorModule::OnButtonClicked()
{
	const FSoftObjectPath widgetAssetPath(
		"/Game/Design/Sequencer/EditorUtilityWidget/EUW_SequencerTool.EUW_SequencerTool"
	);

	UObject* widgetAssetLoaded = widgetAssetPath.TryLoad();
	if ( widgetAssetLoaded == nullptr ) {
		UE_LOG( LogTemp, Warning, TEXT( "Missing Expected widget class at :/Game/Design/Sequencer/EditorUtilityWidget/EUW_SequencerTool.EUW_SequencerTool" ) );
		return;
	}

	UEditorUtilityWidgetBlueprint* widget = Cast<UEditorUtilityWidgetBlueprint>( widgetAssetLoaded );
	if ( widget == nullptr ) {
		UE_LOG( LogTemp, Warning, TEXT( "Couldnt cast /Game/Design/Sequencer/EditorUtilityWidget/EUW_SequencerTool.EUW_SequencerTool to UEditorUtilityWidgetBlueprint" ) );
		return;
	}

	FName widgetID;
	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	EditorUtilitySubsystem->SpawnAndRegisterTabAndGetID( widget, widgetID );
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FEasySequencer_EditorModule, EasySequencer_Editor )