// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widgets/BuildingUIConfig.h"

#include "Building/Building.h"
#include "Localization/GameLocalization.h"

#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"

namespace
{
	// "BP_Wooden_House_C" -> "WoodenHouse", matching the Building.Name.* key suffixes.
	FString LocalizationIdForClass( const UClass& buildingClass )
	{
		FString id = buildingClass.GetName();
		id.RemoveFromEnd( TEXT( "_C" ) );
		id.RemoveFromStart( TEXT( "BP_" ) );
		id.ReplaceInline( TEXT( "_" ), TEXT( "" ) );
		return id;
	}

	FText ResolveBuildingText(
	    const TCHAR* keyPrefix, const FName explicitId, const UClass& buildingClass, const FText& inlineText )
	{
		const FString id = explicitId.IsNone() ? LocalizationIdForClass( buildingClass ) : explicitId.ToString();
		const FString key = FString( keyPrefix ) + id;

		const FStringTableConstPtr table =
		    FStringTableRegistry::Get().FindStringTable( LordsFrontiersLoc::GetTableId() );
		if ( table.IsValid() && table->FindEntry( key ).IsValid() )
		{
			return FText::FromStringTable( LordsFrontiersLoc::GetTableId(), key );
		}
		return inlineText;
	}
} // namespace

FText UBuildingUIConfig::GetBuildingName( TSubclassOf<ABuilding> buildingClass ) const
{
	const FBuildingUIData* data = buildingClass ? BuildingsData.Find( buildingClass ) : nullptr;
	if ( !data )
	{
		return FText::GetEmpty();
	}
	return ResolveBuildingText( TEXT( "Building.Name." ), data->LocalizationId, **buildingClass, data->Name );
}

FText UBuildingUIConfig::GetBuildingDescription( TSubclassOf<ABuilding> buildingClass ) const
{
	const FBuildingUIData* data = buildingClass ? BuildingsData.Find( buildingClass ) : nullptr;
	if ( !data )
	{
		return FText::GetEmpty();
	}
	return ResolveBuildingText(
	    TEXT( "Building.Description." ), data->LocalizationId, **buildingClass, data->Description );
}
