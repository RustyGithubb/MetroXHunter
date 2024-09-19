/*
 * Implemented by Corentin Paya
 */

#include "EasySequencer_Editor.h"
#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

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

	ToolbarSection.AddEntry( FToolMenuEntry::InitToolBarButton(
		TEXT( "MyCustomButtonName" ),
		FExecuteAction::CreateLambda(
			[]() {
				// Simply log for this example
				UE_LOG( LogTemp, Log, TEXT( "Easy Sequencer Triggered" ) );
			} 
		),
		INVTEXT( "ES Button" ),
		INVTEXT( "Easy Sequencer Tool" ),
		FSlateIcon( FAppStyle::GetAppStyleSetName(), "PlayWorld.PlayInCameraLocation" )
	));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FEasySequencer_EditorModule, EasySequencer_Editor )