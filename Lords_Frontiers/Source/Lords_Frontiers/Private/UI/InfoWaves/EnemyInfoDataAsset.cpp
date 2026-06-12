// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InfoWaves/EnemyInfoDataAsset.h"

#include "Localization/GameLocalization.h"
#include "Units/Unit.h"

#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"

namespace
{
	// "BP_Goblin_Wolf_C" -> "GoblinWolf", matching the Enemy.Name.* key suffixes.
	FString LocalizationIdForClass( const UClass& enemyClass )
	{
		FString id = enemyClass.GetName();
		id.RemoveFromEnd( TEXT( "_C" ) );
		id.RemoveFromStart( TEXT( "BP_" ) );
		id.ReplaceInline( TEXT( "_" ), TEXT( "" ) );
		return id;
	}

	FText ResolveEnemyText(
	    const TCHAR* keyPrefix, const FName explicitId, const UClass& enemyClass, const FText& inlineText )
	{
		const FString id = explicitId.IsNone() ? LocalizationIdForClass( enemyClass ) : explicitId.ToString();
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

FText UEnemyInfoDataAsset::GetEnemyName( TSubclassOf<AUnit> enemyClass ) const
{
	const FEnemyUIData* data = enemyClass ? EnemyDataMap.Find( enemyClass ) : nullptr;
	if ( !data )
	{
		return FText::GetEmpty();
	}
	return ResolveEnemyText( TEXT( "Enemy.Name." ), data->LocalizationId, **enemyClass, data->EnemyName );
}

FText UEnemyInfoDataAsset::GetEnemyDescription( TSubclassOf<AUnit> enemyClass ) const
{
	const FEnemyUIData* data = enemyClass ? EnemyDataMap.Find( enemyClass ) : nullptr;
	if ( !data )
	{
		return FText::GetEmpty();
	}
	return ResolveEnemyText(
	    TEXT( "Enemy.Description." ), data->LocalizationId, **enemyClass, data->EnemyDescription );
}
