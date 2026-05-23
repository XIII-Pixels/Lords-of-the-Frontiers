#include "Localization/GameLocalization.h"

#include "Internationalization/StringTable.h"

namespace LordsFrontiersLoc
{
	void Initialize()
	{
		UStringTable* table = LoadObject<UStringTable>(
		    nullptr, TEXT( "/Game/Localization/ST_GameStrings.ST_GameStrings" ) );
		if ( !table )
		{
			UE_LOG( LogTemp, Error,
			    TEXT( "GameLocalization: ST_GameStrings asset not found at "
			          "/Game/Localization/. UI text will render as "
			          "<MISSING STRING TABLE ENTRY>. See Doc/Localization.md." ) );
		}
	}

	void Shutdown()
	{
	}
}
