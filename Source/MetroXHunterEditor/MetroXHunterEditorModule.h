#pragma once

#include "Modules/ModuleManager.h"

class FMetroXHunterEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
