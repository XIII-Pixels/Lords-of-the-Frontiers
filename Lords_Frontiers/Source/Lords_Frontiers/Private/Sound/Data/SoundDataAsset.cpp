#include "sound/Data/SoundDataAsset.h"

const FSoundEntry* USoundDataAsset::FindByTag( const FGameplayTag& tag )
{
	return Entries_.Find( tag );
}
