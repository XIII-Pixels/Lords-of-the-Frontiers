#pragma once

#include "CoreMinimal.h"

class AUnit;

DECLARE_MULTICAST_DELEGATE_OneParam( FOnUnitDied, AUnit* );

struct LORDS_FRONTIERS_API FUnitEvents
{
	static FOnUnitDied OnUnitDied;
};
