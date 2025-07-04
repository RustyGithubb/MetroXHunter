#include "MetroXHunterEditorModule.h"
#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "DialogueManagerComponentDetails.h"

#define LOCTEXT_NAMESPACE "FMetroXHunterEditorModule"

void FMetroXHunterEditorModule::StartupModule()
{
    FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
    PropertyModule.RegisterCustomClassLayout( "DialogueManagerComponent", FOnGetDetailCustomizationInstance::CreateStatic( &FDialogueManagerComponentDetails::MakeInstance ) );
    PropertyModule.NotifyCustomizationModuleChanged();
}

void FMetroXHunterEditorModule::ShutdownModule()
{
    if ( FModuleManager::Get().IsModuleLoaded( "PropertyEditor" ) )
    {
        FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
        PropertyModule.UnregisterCustomClassLayout( "DialogueManagerComponent" );
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE( FMetroXHunterEditorModule, MetroXHunterEditor )
