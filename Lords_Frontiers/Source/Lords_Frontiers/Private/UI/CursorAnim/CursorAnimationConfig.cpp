#include "UI/CursorAnim/CursorAnimationConfig.h"

const FCursorAnimationEntry* UCursorAnimationConfig::FindByTag( const FGameplayTag& tag ) const
{
	for ( const FCursorAnimationEntry& entry : Animations_ )
	{
		if ( entry.Tag == tag )
		{
			return &entry;
		}
	}
	return nullptr;
}
