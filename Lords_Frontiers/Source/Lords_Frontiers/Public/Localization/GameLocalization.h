#pragma once

#include "CoreMinimal.h"

namespace LordsFrontiersLoc
{
	// Object path of the ST_GameStrings UStringTable asset. The asset lives at
	// /Game/Localization/ST_GameStrings and must be created in the editor
	// (see Doc/Localization.md). UE auto-registers loaded UStringTable assets
	// in FStringTableRegistry under this path, so LF_LOC can look them up.
	inline FName GetTableId()
	{
		static const FName Id( TEXT( "/Game/Localization/ST_GameStrings.ST_GameStrings" ) );
		return Id;
	}

	// Native culture of the ST_GameStrings source strings.
	inline const TCHAR* GetNativeCulture()
	{
		return TEXT( "ru" );
	}

	// Force-loads the StringTable asset at module startup so its keys become resolvable
	// before any widget initializes, then registers the culture columns of
	// Content/Localization/ST_GameStrings.csv (Key,SourceString,en,...) as polyglot text
	// data — no Localization Dashboard / .locres step is needed for those translations.
	// Call once from StartupModule().
	void Initialize();

	// No-op kept for symmetry; the engine handles asset/registry teardown.
	void Shutdown();
}

// Convenience macro for C++ call sites:
//   FText t = LF_LOC( "HUD.Day" );
// For formatted strings use FText::Format( LF_LOC("HUD.Day"), FText::AsNumber(wave) ).
#define LF_LOC( KeyText ) ::FText::FromStringTable( ::LordsFrontiersLoc::GetTableId(), TEXT( KeyText ) )
