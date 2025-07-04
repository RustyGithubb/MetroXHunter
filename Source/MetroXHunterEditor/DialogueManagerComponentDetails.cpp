#include "DialogueManagerComponentDetails.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Engine/DataTable.h"
#include "DialogueManagerComponent.h"
#include "Editor/PropertyEditor/Public/PropertyEditing.h"

#define LOCTEXT_NAMESPACE "DialogueManagerComponentDetails"

TSharedRef<IDetailCustomization> FDialogueManagerComponentDetails::MakeInstance()
{
	return MakeShareable( new FDialogueManagerComponentDetails );
}

void FDialogueManagerComponentDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	// Retrieve the handle for the "DialogueNamespace" property of your component
	TSharedRef<IPropertyHandle> NamespaceHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogueManagerComponent, DialogueNamespace));

	// Retrieve the handle for the "Dialogue" property (the DataTable)
	TSharedPtr<IPropertyHandle> DataTableHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogueManagerComponent, Dialogue));

	// Populate the list of options from the DataTable
	RefreshNamespaceOptions(DetailBuilder);

	// Hide the default widget for DialogueNamespace
	DetailBuilder.HideProperty(NamespaceHandle);

	// Add a custom row to display the combo box in the "Dialogue" category
	IDetailCategoryBuilder& DialogueCategory = DetailBuilder.EditCategory("Dialogue");
	DialogueCategory.AddCustomRow(LOCTEXT("DialogueNamespaceFilter", "Dialogue Namespace"))
		.NameContent()
		[
			NamespaceHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(250)
		[
			SNew(SComboBox<TSharedPtr<FName>>)
			.OptionsSource(&NamespaceOptions)
			.OnGenerateWidget_Lambda([](TSharedPtr<FName> InOption)
			{
				return SNew(STextBlock).Text(FText::FromName(*InOption));
			})
			.OnSelectionChanged_Lambda([NamespaceHandle](TSharedPtr<FName> NewValue, ESelectInfo::Type)
			{
				if (NewValue.IsValid())
				{
					NamespaceHandle->SetValue(*NewValue);
				}
			})
			[
				SNew(STextBlock)
				.Text_Lambda([NamespaceHandle]()
				{
					FName CurrentValue;
					if (NamespaceHandle->GetValue(CurrentValue) == FPropertyAccess::Success)
					{
						return FText::FromName(CurrentValue);
					}
					return LOCTEXT("SelectNamespace", "Select Namespace");
				})
			]
		];
}

void FDialogueManagerComponentDetails::RefreshNamespaceOptions(IDetailLayoutBuilder& DetailBuilder)
{
	NamespaceOptions.Empty();

	// Retrieve the handle for the "Dialogue" property
	TSharedPtr<IPropertyHandle> DataTableHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogueManagerComponent, Dialogue));
	if (!DataTableHandle.IsValid())
	{
		return;
	}

	UObject* DataTableObject = nullptr;
	if (DataTableHandle->GetValue(DataTableObject) != FPropertyAccess::Success || !DataTableObject)
	{
		return;
	}

	UDataTable* DataTable = Cast<UDataTable>(DataTableObject);
	if (!DataTable)
	{
		return;
	}

	// Retrieve all row names from the DataTable
	static const FString ContextString(TEXT("DialogueContext"));
	TArray<FName> RowNames = DataTable->GetRowNames();

	// Use a TSet to avoid duplicates
	TSet<FName> UniqueNamespaces;
	for (const FName& RowName : RowNames)
	{
		FDialogue* Row = DataTable->FindRow<FDialogue>(RowName, ContextString);
		if (Row)
		{
			UniqueNamespaces.Add(Row->DialogueNamespace);
		}
	}

	// Populate the options array with the unique values
	for (const FName& NamespaceValue : UniqueNamespaces)
	{
		NamespaceOptions.Add(MakeShareable(new FName(NamespaceValue)));
	}
}

#undef LOCTEXT_NAMESPACE

