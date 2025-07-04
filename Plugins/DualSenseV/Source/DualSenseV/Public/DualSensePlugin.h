#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "IInputDeviceModule.h"

class FDualSenseInputDevice;

DECLARE_LOG_CATEGORY_EXTERN( LogDualSensePlugin, Log, All );

class DUALSENSEV_API FDualSensePlugin : public IInputDeviceModule
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr<IInputDevice> CreateInputDevice(
		const TSharedRef<FGenericApplicationMessageHandler>& MessageHandler
	) override;

	TSharedPtr<FDualSenseInputDevice> GetInputDevice() const;

	static inline FDualSensePlugin& GetChecked()
	{
		return FModuleManager::LoadModuleChecked<FDualSensePlugin>( "DualSenseV" );
	}

private:
	TSharedPtr<FDualSenseInputDevice> InputDevice = nullptr;
};
