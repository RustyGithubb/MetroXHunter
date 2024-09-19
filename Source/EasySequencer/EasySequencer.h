/*
 * Implemented by Corentin Paya
 */

#include "Modules/ModuleManager.h"
#include "ToolMenus.h"

/*
 * Note: It seems that we cannot open Unreal if we delete the EasySequencer.cpp,
 * even when moving the "IMPLEMENT_MODULE[...]" to the header file.
 *
 * That's why we keep this empty class, only for unreal to initialize the module properly.
 */
class FEasySequencerModule : public IModuleInterface
{
public:
	void StartupModule() override;
	void ShutdownModule() override;
};