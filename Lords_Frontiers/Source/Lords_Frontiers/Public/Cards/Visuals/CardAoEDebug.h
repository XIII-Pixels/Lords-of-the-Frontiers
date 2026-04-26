#pragma once

#include "CoreMinimal.h"

class UObject;

namespace CardAoEDebug
{
	LORDS_FRONTIERS_API void DrawRadius(
		const UObject* worldContextObject,
		const FVector& center,
		float radius,
		float duration = 1.0f,
		const FColor& color = FColor::Red );
}
