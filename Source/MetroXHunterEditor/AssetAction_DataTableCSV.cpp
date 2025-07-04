#include "AssetAction_DataTableCSV.h"
#include "Engine/DataTable.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/UnrealType.h"         // for TFieldIterator
#include "Kismet/KismetStringLibrary.h"  // for potential string functions
#include "Kismet/DataTableFunctionLibrary.h" // (if needed)

// For file dialog boxes
#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"

// To access selected assets in the Content Browser
#include "EditorUtilityLibrary.h"

// Include for dynamic localization creation
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"

//-------------------------------------------------------------------
// Helper function: EscapeCSVValue
//-------------------------------------------------------------------
FString UAssetAction_DataTableCSV::EscapeCSVValue( const FString& InValue )
{
	// Check if the value contains commas, line breaks, or quotes.
	bool bNeedsQuotes = InValue.Contains( TEXT( "," ) ) || InValue.Contains( TEXT( "\n" ) ) || InValue.Contains( TEXT( "\"" ) );
	FString EscapedValue = InValue;
	if ( bNeedsQuotes )
	{
		// Double any internal quotes.
		EscapedValue.ReplaceInline( TEXT( "\"" ), TEXT( "\"\"" ) );
		// Surround with quotes.
		EscapedValue = FString::Printf( TEXT( "\"%s\"" ), *EscapedValue );
	}
	return EscapedValue;
}

//-------------------------------------------------------------------
// Helper function: ExtractLocalizationInfo
//-------------------------------------------------------------------
bool UAssetAction_DataTableCSV::ExtractLocalizationInfo( const FText& InText, FString& OutNamespace, FString& OutKey, FString& OutDisplayText ) const
{
	// Convert the FText to a display string.
	OutDisplayText = InText.ToString();

#if WITH_EDITOR
	// Retrieve the optional namespace and key via FTextInspector.
	const TOptional<FString> Namespace = FTextInspector::GetNamespace( InText );
	const TOptional<FString> Key = FTextInspector::GetKey( InText );
	OutNamespace = Namespace.IsSet() ? Namespace.GetValue() : TEXT( "" );
	OutKey = Key.IsSet() ? Key.GetValue() : TEXT( "" );
#else
	OutNamespace = TEXT( "" );
	OutKey = TEXT( "" );
#endif

	return true;
}

//-------------------------------------------------------------------
// Helper function: ParseCSVLine
// This function parses a CSV line taking into account quoted fields.
//-------------------------------------------------------------------
static TArray<FString> ParseCSVLine( const FString& Line )
{
	TArray<FString> Fields;
	FString CurrentField;
	bool bInQuotes = false;

	for ( int32 i = 0; i < Line.Len(); i++ )
	{
		TCHAR Char = Line[i];
		if ( Char == '\"' )
		{
			// If in quotes and the next character is also a quote, it's an escaped quote.
			if ( bInQuotes && i + 1 < Line.Len() && Line[i + 1] == '\"' )
			{
				CurrentField.AppendChar( '\"' );
				i++; // Skip the next quote.
			}
			else
			{
				bInQuotes = !bInQuotes;
			}
		}
		else if ( Char == ',' && !bInQuotes )
		{
			// End of field.
			Fields.Add( CurrentField );
			CurrentField.Empty();
		}
		else
		{
			CurrentField.AppendChar( Char );
		}
	}
	// Add the final field.
	Fields.Add( CurrentField );
	return Fields;
}

//-------------------------------------------------------------------
// Helper function: ExportDataTableToCustomCSV
//-------------------------------------------------------------------
FString UAssetAction_DataTableCSV::ExportDataTableToCustomCSV( UDataTable* DataTable ) const
{
	FString CSVContent;
	if ( !DataTable || !DataTable->RowStruct )
	{
		return CSVContent;
	}

	// --- 1. Create the CSV header
	TArray<FString> HeaderColumns;
	HeaderColumns.Add( TEXT( "RowName" ) ); // Row name

	// Iterate over the properties of the row struct.
	for ( TFieldIterator<FProperty> It( DataTable->RowStruct ); It; ++It )
	{
		FProperty* Property = *It;
		// For FText properties, create 3 columns.
		if ( Property->IsA( FTextProperty::StaticClass() ) )
		{
			HeaderColumns.Add( Property->GetName() + TEXT( "_Display" ) );
			HeaderColumns.Add( Property->GetName() + TEXT( "_Namespace" ) );
			HeaderColumns.Add( Property->GetName() + TEXT( "_Key" ) );
		}
		// For enum properties, export as a single column (display string).
		else if ( FEnumProperty* EnumProp = CastField<FEnumProperty>( Property ) )
		{
			HeaderColumns.Add( Property->GetName() );
		}
		else
		{
			HeaderColumns.Add( Property->GetName() );
		}
	}
	CSVContent += FString::Join( HeaderColumns, TEXT( "," ) ) + LINE_TERMINATOR;

	// --- 2. Iterate over the rows of the DataTable using GetRowNames() and FindRow().
	TArray<FName> RowNames = DataTable->GetRowNames();
	for ( const FName& RowName : RowNames )
	{
		// Retrieve the row; assuming rows derive from FTableRowBase.
		FTableRowBase* Row = DataTable->FindRow<FTableRowBase>( RowName, TEXT( "ExportCSV" ), false );
		if ( !Row )
		{
			continue;
		}
		uint8* RowData = reinterpret_cast<uint8*>( Row );

		TArray<FString> RowValues;
		// Add the row name.
		RowValues.Add( RowName.ToString() );

		// Iterate over the properties of the row struct.
		for ( TFieldIterator<FProperty> It( DataTable->RowStruct ); It; ++It )
		{
			FProperty* Property = *It;
			// Handle FText properties.
			if ( FTextProperty* TextProp = CastField<FTextProperty>( Property ) )
			{
				FText* TextValue = TextProp->GetPropertyValuePtr_InContainer( RowData );
				FString DisplayText, Namespace, Key;
				ExtractLocalizationInfo( *TextValue, Namespace, Key, DisplayText );
				RowValues.Add( EscapeCSVValue( DisplayText ) );
				RowValues.Add( EscapeCSVValue( Namespace ) );
				RowValues.Add( EscapeCSVValue( Key ) );
			}
			// Handle enum properties.
			else if ( FEnumProperty* EnumProp = CastField<FEnumProperty>( Property ) )
			{
				FNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
				int64 EnumValue = UnderlyingProp->GetSignedIntPropertyValue( EnumProp->ContainerPtrToValuePtr<void>( RowData ) );
				const UEnum* EnumPtr = EnumProp->GetEnum();
				FString EnumString = EnumPtr->GetDisplayNameTextByValue( EnumValue ).ToString();
				RowValues.Add( EscapeCSVValue( EnumString ) );
			}
			// For other types, export directly as text.
			else
			{
				FString PropValue;
				Property->ExportText_Direct( PropValue, Property->ContainerPtrToValuePtr<void>( RowData ), nullptr, nullptr, PPF_None );
				RowValues.Add( EscapeCSVValue( PropValue ) );
			}
		}
		CSVContent += FString::Join( RowValues, TEXT( "," ) ) + LINE_TERMINATOR;
	}

	return CSVContent;
}

//-------------------------------------------------------------------
// Helper function: ImportDataTableFromCustomCSV
//-------------------------------------------------------------------
void UAssetAction_DataTableCSV::ImportDataTableFromCustomCSV( UDataTable* DataTable, const FString& CSVContent ) const
{
	if ( !DataTable || !DataTable->RowStruct )
	{
		return;
	}

	// Split the CSV content into lines.
	TArray<FString> Lines;
	CSVContent.ParseIntoArrayLines( Lines );
	if ( Lines.Num() < 2 )
	{
		UE_LOG( LogTemp, Error, TEXT( "CSV is empty or invalid." ) );
		return;
	}

	// The first line is the header (not used here).
	TArray<FString> Headers = ParseCSVLine( Lines[0] );

	// For each data line:
	for ( int32 i = 1; i < Lines.Num(); ++i )
	{
		TArray<FString> Columns = ParseCSVLine( Lines[i] );
		if ( Columns.Num() < 1 )
		{
			continue;
		}

		// The first column is the row name.
		FName RowName( *Columns[0] );

		// Retrieve the row using FindRow.
		FTableRowBase* Row = DataTable->FindRow<FTableRowBase>( RowName, TEXT( "ImportCSV" ), false );
		if ( !Row )
		{
			UE_LOG( LogTemp, Warning, TEXT( "Row %s not found in DataTable." ), *RowName.ToString() );
			continue;
		}
		uint8* RowData = reinterpret_cast<uint8*>( Row );
		int32 ColIndex = 1; // Start after the row name

		// Iterate over the properties of the row struct.
		for ( TFieldIterator<FProperty> It( DataTable->RowStruct ); It; ++It )
		{
			FProperty* Property = *It;
			// Handle FText properties.
			if ( FTextProperty* TextProp = CastField<FTextProperty>( Property ) )
			{
				// Expect 3 columns for an FText property.
				if ( Columns.IsValidIndex( ColIndex + 2 ) )
				{
					FString DisplayText = Columns[ColIndex];
					// Trim extra enclosing quotes if present.
					if ( DisplayText.StartsWith( TEXT( "\"" ) ) && DisplayText.EndsWith( TEXT( "\"" ) ) )
					{
						DisplayText = DisplayText.Mid( 1, DisplayText.Len() - 2 );
					}
					FString Namespace = Columns[ColIndex + 1];
					FString Key = Columns[ColIndex + 2];
					FText NewText = FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText( *DisplayText, *Namespace, *Key );
					TextProp->SetPropertyValue_InContainer( RowData, NewText );
				}
				ColIndex += 3;
			}
			// Handle enum properties.
			else if ( FEnumProperty* EnumProp = CastField<FEnumProperty>( Property ) )
			{
				if ( Columns.IsValidIndex( ColIndex ) )
				{
					FString EnumDisplayString = Columns[ColIndex];
					const UEnum* EnumPtr = EnumProp->GetEnum();
					int64 FoundValue = 0;
					bool bFound = false;
					// Iterate through the enum entries (skipping hidden or _MAX values if necessary).
					for ( int32 j = 0; j < EnumPtr->NumEnums() - 1; j++ )
					{
						FString CurrDisplay = EnumPtr->GetDisplayNameTextByIndex( j ).ToString();
						if ( CurrDisplay.Equals( EnumDisplayString, ESearchCase::IgnoreCase ) )
						{
							FoundValue = EnumPtr->GetValueByIndex( j );
							bFound = true;
							break;
						}
					}
					if ( bFound )
					{
						FNumericProperty* UnderlyingProp = EnumProp->GetUnderlyingProperty();
						UnderlyingProp->SetIntPropertyValue( EnumProp->ContainerPtrToValuePtr<void>( RowData ), FoundValue );
					}
					ColIndex += 1;
				}
			}
			// For other types, use ImportText.
			else
			{
				if ( Columns.IsValidIndex( ColIndex ) )
				{
					Property->ImportText( *Columns[ColIndex], Property->ContainerPtrToValuePtr<void>( RowData ), PPF_None, nullptr );
					ColIndex += 1;
				}
			}
		}
	}

	// Mark the DataTable as modified to save changes.
	DataTable->MarkPackageDirty();
}

//-------------------------------------------------------------------
// Method ExportCSV (called from the editor)
//-------------------------------------------------------------------
void UAssetAction_DataTableCSV::ExportCSV()
{
	// Retrieve the assets selected in the editor.
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	for ( UObject* Asset : SelectedAssets )
	{
		UDataTable* DataTable = Cast<UDataTable>( Asset );
		if ( DataTable )
		{
			// Generate the custom CSV content.
			FString CSVContent = ExportDataTableToCustomCSV( DataTable );

			// Open a file dialog to save the CSV file.
			FString DefaultPath = FPaths::ProjectDir();
			FString DefaultFile = DataTable->GetName() + TEXT( ".csv" );
			TArray<FString> OutFilenames;
			IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
			if ( DesktopPlatform )
			{
				bool bFileSelected = DesktopPlatform->SaveFileDialog(
					FSlateApplication::Get().FindBestParentWindowHandleForDialogs( nullptr ),
					TEXT( "Save CSV" ),
					DefaultPath,
					DefaultFile,
					TEXT( "CSV Files (*.csv)|*.csv" ),
					EFileDialogFlags::None,
					OutFilenames
				);

				if ( bFileSelected && OutFilenames.Num() > 0 )
				{
					FString SavePath = OutFilenames[0];
					if ( FFileHelper::SaveStringToFile( CSVContent, *SavePath ) )
					{
						UE_LOG( LogTemp, Log, TEXT( "DataTable %s successfully exported to %s" ), *DataTable->GetName(), *SavePath );
					}
					else
					{
						UE_LOG( LogTemp, Error, TEXT( "Error saving CSV file for DataTable %s" ), *DataTable->GetName() );
					}
				}
			}
		}
	}
}

//-------------------------------------------------------------------
// Method ImportCSV (called from the editor)
//-------------------------------------------------------------------
void UAssetAction_DataTableCSV::ImportCSV()
{
	// Retrieve the assets selected in the editor.
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	for ( UObject* Asset : SelectedAssets )
	{
		UDataTable* DataTable = Cast<UDataTable>( Asset );
		if ( DataTable )
		{
			// Open a file dialog to select the CSV file to import.
			FString DefaultPath = FPaths::ProjectDir();
			TArray<FString> OutFilenames;
			IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
			if ( DesktopPlatform )
			{
				bool bFileSelected = DesktopPlatform->OpenFileDialog(
					FSlateApplication::Get().FindBestParentWindowHandleForDialogs( nullptr ),
					TEXT( "Open CSV" ),
					DefaultPath,
					TEXT( "" ),
					TEXT( "CSV Files (*.csv)|*.csv" ),
					EFileDialogFlags::None,
					OutFilenames
				);

				if ( bFileSelected && OutFilenames.Num() > 0 )
				{
					FString CSVPath = OutFilenames[0];
					FString CSVContent;
					if ( FFileHelper::LoadFileToString( CSVContent, *CSVPath ) )
					{
						// Import the custom CSV into the DataTable.
						ImportDataTableFromCustomCSV( DataTable, CSVContent );
						UE_LOG( LogTemp, Log, TEXT( "DataTable %s successfully imported from %s" ), *DataTable->GetName(), *CSVPath );
					}
					else
					{
						UE_LOG( LogTemp, Error, TEXT( "Error loading CSV file %s for DataTable %s" ), *CSVPath, *DataTable->GetName() );
					}
				}
			}
		}
	}
}
