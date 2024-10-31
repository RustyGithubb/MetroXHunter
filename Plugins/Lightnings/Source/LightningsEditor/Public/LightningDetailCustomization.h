#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FLightningDetailCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// Run button callback function
	FReply OnRunButtonClicked();
	// Stop button callback function
	FReply OnStopButtonClicked();
	// Reset button callback function
	FReply OnResetButtonClicked();

	// Hides unused variables in attachment params
	void ConfigureAttachmentParams(IDetailLayoutBuilder& DetailBuilder, FName AttachmentName);

	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
};
