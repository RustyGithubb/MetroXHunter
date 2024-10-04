/*
 * Implemented by Arthur Cathelain (arkaht)
 */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FHeatmapperModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
