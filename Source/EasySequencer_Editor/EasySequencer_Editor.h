/*
 * Implemented by Corentin Paya
 */

#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

class FEasySequencer_EditorModule : public IModuleInterface
{
public:
    void StartupModule() override;
    void ShutdownModule() override;

private:
    void RegisterMenuExtensions();
};