#include "LightningsEditor.h"
#include "Framework/LightningEmitter.h"
#include "LightningDetailCustomization.h"

#define LOCTEXT_NAMESPACE "FLightningsEditorModule"

void FLightningsEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(ALightningEmitter::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FLightningDetailCustomization::MakeInstance));
}

void FLightningsEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLightningsEditorModule, LightningsEditor)
