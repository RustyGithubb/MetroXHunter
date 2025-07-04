/*
 * Implemented by BARRAU Benoit
 */

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AssetAction_DataTableCSV.generated.h"

 /**
  * Utility to export/import a DataTable in CSV format with localization info separated.
  *
  * For each FText property in a DataTable row, the CSV will contain three columns:
  *  - <PropertyName>_Display : the text to display (editable)
  *  - <PropertyName>_Namespace : the localization namespace (hidden in CSV editor)
  *  - <PropertyName>_Key     : the localization key (hidden in CSV editor)
  */
UCLASS()
class UAssetAction_DataTableCSV : public UAssetActionUtility
{
	GENERATED_BODY()

public:
	/** Exports the selected DataTable to a custom CSV file */
	UFUNCTION( CallInEditor, Category = "DataTable CSV" )
	void ExportCSV();

	/** Imports CSV data into the selected DataTable */
	UFUNCTION( CallInEditor, Category = "DataTable CSV" )
	void ImportCSV();

	static FString EscapeCSVValue( const FString& InValue );

private:
	/** Extracts the localization info from an FText.
		This example uses FText::ToString() for the display text.
		For Namespace and Key, if possible use FTextInspector (editor-only) or leave empty.
	*/
	bool ExtractLocalizationInfo( const FText& InText, FString& OutNamespace, FString& OutKey, FString& OutDisplayText ) const;

	/** Generates a custom CSV string from the given DataTable */
	FString ExportDataTableToCustomCSV( class UDataTable* DataTable ) const;

	/** Imports CSV content into the given DataTable */
	void ImportDataTableFromCustomCSV( class UDataTable* DataTable, const FString& CSVContent ) const;
};
