#pragma once

#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"

class IDetailLayoutBuilder;

class FDialogueManagerComponentDetails : public IDetailCustomization
{
public:
	// Factory method for this detail customization instance.
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface.
	virtual void CustomizeDetails( IDetailLayoutBuilder& DetailBuilder ) override;

private:
	// Array to store the available namespace options.
	TArray<TSharedPtr<FName>> NamespaceOptions;

	// Helper function to refresh the list of namespace options from the DataTable.
	void RefreshNamespaceOptions( IDetailLayoutBuilder& DetailBuilder );
};
