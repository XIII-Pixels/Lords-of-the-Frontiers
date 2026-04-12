#pragma once

#include "Units/Unit.h"

#include "CoreMinimal.h"

#include "FlyingUnit.generated.h"


UCLASS( Abstract, Blueprintable )
class LORDS_FRONTIERS_API AFlyingUnit : public AUnit
{
	GENERATED_BODY()

public:
	AFlyingUnit();
};