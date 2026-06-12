#include "Localization/GameLocalization.h"

#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/PolyglotTextData.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/Csv/CsvParser.h"

namespace
{
	// Combined CSV layout: Key,SourceString,<culture>,<culture>,...
	// The first two columns are what the editor imports into ST_GameStrings;
	// every extra column holds translations for the culture named in its header.
	constexpr int32 KeyColumn = 0;
	constexpr int32 SourceColumn = 1;
	constexpr int32 FirstCultureColumn = 2;

	TArray<FPolyglotTextData> LoadCsvTranslations( const FString& filePath, const FStringTableConstRef& stringTable )
	{
		TArray<FPolyglotTextData> result;

		FString csvContent;
		if ( !FFileHelper::LoadFileToString( csvContent, *filePath ) )
		{
			UE_LOG(
			    LogTemp, Warning, TEXT( "GameLocalization: cannot read %s, translations not registered" ), *filePath
			);
			return result;
		}

		const FCsvParser parser( csvContent );
		const FCsvParser::FRows& rows = parser.GetRows();
		if ( rows.IsEmpty() )
		{
			return result;
		}

		const FString fileName = FPaths::GetCleanFilename( filePath );

		// Map culture columns from the header row.
		const TArray<const TCHAR*>& header = rows[0];
		TMap<int32, FString> cultureColumns;
		for ( int32 column = FirstCultureColumn; column < header.Num(); ++column )
		{
			const FString culture = FString( header[column] ).TrimStartAndEnd().ToLower();
			if ( culture.IsEmpty() || culture == LordsFrontiersLoc::GetNativeCulture() )
			{
				continue;
			}
			if ( !FInternationalization::Get().GetCulture( culture ) )
			{
				UE_LOG(
				    LogTemp, Warning, TEXT( "GameLocalization: %s: unknown culture column '%s', skipped" ), *fileName,
				    header[column]
				);
				continue;
			}
			cultureColumns.Add( column, culture );
		}

		if ( cultureColumns.IsEmpty() )
		{
			UE_LOG( LogTemp, Warning, TEXT( "GameLocalization: %s has no culture columns" ), *fileName );
			return result;
		}

		for ( int32 rowIndex = 1; rowIndex < rows.Num(); ++rowIndex )
		{
			const TArray<const TCHAR*>& row = rows[rowIndex];
			if ( row.Num() <= SourceColumn )
			{
				continue;
			}

			const FString key = row[KeyColumn];
			if ( key.IsEmpty() )
			{
				continue;
			}

			const FStringTableEntryConstPtr entry = stringTable->FindEntry( key );
			if ( !entry.IsValid() )
			{
				UE_LOG(
				    LogTemp, Warning,
				    TEXT( "GameLocalization: %s: key '%s' is missing in the string table asset — "
				          "reimport the CSV into it" ),
				    *fileName, *key
				);
				continue;
			}

			const FString& nativeString = entry->GetSourceString();
			if ( nativeString.IsEmpty() )
			{
				continue;
			}
			if ( nativeString != row[SourceColumn] )
			{
				UE_LOG(
				    LogTemp, Warning,
				    TEXT( "GameLocalization: %s: source string for '%s' differs from the string table "
				          "asset — reimport the CSV into it" ),
				    *fileName, *key
				);
			}

			FPolyglotTextData data(
			    ELocalizedTextSourceCategory::Game, stringTable->GetNamespace(), key, nativeString,
			    LordsFrontiersLoc::GetNativeCulture()
			);

			bool hasTranslation = false;
			for ( const TPair<int32, FString>& cultureColumn : cultureColumns )
			{
				if ( row.Num() <= cultureColumn.Key )
				{
					continue;
				}
				// An empty cell simply keeps the native string.
				const FString translation = row[cultureColumn.Key];
				if ( translation.IsEmpty() )
				{
					continue;
				}
				data.AddLocalizedString( cultureColumn.Value, translation );
				hasTranslation = true;
			}

			if ( hasTranslation )
			{
				result.Add( MoveTemp( data ) );
			}
		}

		return result;
	}

	// Force-loads a StringTable asset and registers the culture columns of its companion
	// CSV as runtime translations, so the Localization Dashboard / .locres pipeline is not
	// required for them.
	void RegisterTableTranslations( const TCHAR* assetPath, const TCHAR* csvRelativePath )
	{
		UStringTable* table = LoadObject<UStringTable>( nullptr, assetPath );
		if ( !table )
		{
			UE_LOG(
			    LogTemp, Error,
			    TEXT( "GameLocalization: StringTable asset %s not found. Texts bound to it will "
			          "render as <MISSING STRING TABLE ENTRY>. See Doc/Localization.md." ),
			    assetPath
			);
			return;
		}

		const FString csvPath = FPaths::ProjectContentDir() / csvRelativePath;

		TArray<FPolyglotTextData> translations = LoadCsvTranslations( csvPath, table->GetStringTable() );
		if ( translations.IsEmpty() )
		{
			UE_LOG(
			    LogTemp, Warning,
			    TEXT( "GameLocalization: %s: no translations registered — texts of this table will "
			          "not switch languages. Check that the CSV was imported into %s (Import from "
			          "CSV) and that its culture columns are filled." ),
			    *FPaths::GetCleanFilename( csvPath ), assetPath
			);
			return;
		}

		FTextLocalizationManager::Get().RegisterPolyglotTextData( translations );
		UE_LOG(
		    LogTemp, Log, TEXT( "GameLocalization: %s: registered CSV translations for %d keys" ),
		    *FPaths::GetCleanFilename( csvPath ), translations.Num()
		);
	}
} // namespace

namespace LordsFrontiersLoc
{
	void Initialize()
	{
		// UI strings: LF_LOC call sites and WBP bindings.
		RegisterTableTranslations(
		    TEXT( "/Game/Localization/ST_GameStrings.ST_GameStrings" ),
		    TEXT( "Localization/ST_GameStrings.csv" ) );

		// Card names/descriptions (Card.Name.* / Card.Description.*), bound on
		// UCardDataAsset FText fields — see Doc/CardLocalization.md.
		RegisterTableTranslations(
		    TEXT( "/Game/Localization/ST_Cards.ST_Cards" ), TEXT( "Localization/ST_Cards.csv" ) );
	}

	void Shutdown()
	{
	}
}
