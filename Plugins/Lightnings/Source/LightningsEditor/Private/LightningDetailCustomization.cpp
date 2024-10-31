#include "LightningDetailCustomization.h"

#include "Framework/LightningEmitter.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Input/Reply.h"

TSharedRef<IDetailCustomization> FLightningDetailCustomization::MakeInstance()
{
	return MakeShareable(new FLightningDetailCustomization);
}

void FLightningDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

	/**
	* Check all customized objects to have a valid world
	*/
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (Object.IsValid())
		{
			ALightningEmitter* Emitter = Cast<ALightningEmitter>(Object.Get());
			if (Emitter && Emitter->GetWorld())
			{
				continue;
			}
		}
		return;
	}

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Preview", FText::GetEmpty(), ECategoryPriority::Important);

	Category.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.VAlign(EVerticalAlignment::VAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.Text(FText::FromString("Run"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.DesiredSizeScale(1.5f)
				.OnClicked_Raw(this, &FLightningDetailCustomization::OnRunButtonClicked)
			]
			+ SHorizontalBox::Slot()
			.Padding(8.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Stop"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.DesiredSizeScale(1.5f)
				.OnClicked_Raw(this, &FLightningDetailCustomization::OnStopButtonClicked)
			]
			+ SHorizontalBox::Slot()
			.Padding(0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(FText::FromString("Reset"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				.DesiredSizeScale(1.5f)
				.OnClicked_Raw(this, &FLightningDetailCustomization::OnResetButtonClicked)
			]
		];

	ConfigureAttachmentParams(DetailBuilder, "SourceAttachment");
	ConfigureAttachmentParams(DetailBuilder, "TargetAttachment");
}

FReply FLightningDetailCustomization::OnRunButtonClicked()
{
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (Object.IsValid())
		{
			ALightningEmitter* Emitter = Cast<ALightningEmitter>(Object.Get());
			if (Emitter)
			{
				Emitter->ActivateEmitter_Implementation();
			}
		}
	}
	return FReply::Handled();
}

FReply FLightningDetailCustomization::OnStopButtonClicked()
{
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (Object.IsValid())
		{
			ALightningEmitter* Emitter = Cast<ALightningEmitter>(Object.Get());
			if (Emitter)
			{
				Emitter->DeactivateEmitter_Implementation();
			}
		}
	}
	return FReply::Handled();
}

FReply FLightningDetailCustomization::OnResetButtonClicked()
{
	for (TWeakObjectPtr<UObject> Object : CustomizedObjects)
	{
		if (Object.IsValid())
		{
			ALightningEmitter* Emitter = Cast<ALightningEmitter>(Object.Get());
			if (Emitter)
			{
				Emitter->ResetGenerator_Implementation();
			}
		}
	}
	return FReply::Handled();
}

void FLightningDetailCustomization::ConfigureAttachmentParams(IDetailLayoutBuilder& DetailBuilder, FName AttachmentName)
{
	const TSharedRef<IPropertyHandle> AttachmentProperty = DetailBuilder.GetProperty(AttachmentName);

	/**
	* Attach position params
	*/

	EAttachType Type;
	FPropertyAccess::Result Res = AttachmentProperty->GetChildHandle("Type")->GetValue((uint8&)Type);

	if (Res != FPropertyAccess::Result::Fail)
	{
		if (Res == FPropertyAccess::Result::MultipleValues || Type == EAttachType::AT_Self)
		{
			DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("Location"));
			DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("ActorAttachParams"));
		}
		else
		{
			switch (Type)
			{
			case EAttachType::AT_Location:
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("ActorAttachParams"));
				break;
			case EAttachType::AT_Actor:
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("Location"));
				break;
			default:
				break;
			}
		}
	}

	/**
	* Attach volume params
	*/

	EAttachVolumeType AttachVolumeType;
	Res = AttachmentProperty->GetChildHandle("AttachVolumeType")->GetValue((uint8&)AttachVolumeType);

	if (Res != FPropertyAccess::Result::Fail)
	{
		if (Res == FPropertyAccess::Result::MultipleValues || AttachVolumeType == EAttachVolumeType::AV_Point)
		{
			DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeBox"));
			DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeCylinder"));
			DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeSphere"));
		}
		else
		{
			switch (AttachVolumeType)
			{
			case EAttachVolumeType::AV_Box:
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeCylinder"));
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeSphere"));
				break;
			case EAttachVolumeType::AV_Cylinder:
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeBox"));
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeSphere"));
				break;
			case EAttachVolumeType::AV_Sphere:
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeBox"));
				DetailBuilder.HideProperty(AttachmentProperty->GetChildHandle("AttachVolumeCylinder"));
				break;
			default:
				break;
			}
		}
	}
}
