#include "Cards/Visuals/CardAoEDebug.h"

#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

namespace CardAoEDebug
{
	void DrawRadius(
		const UObject* worldContextObject,
		const FVector& center,
		float radius,
		float duration,
		const FColor& color )
	{
#if ENABLE_DRAW_DEBUG
		if ( !worldContextObject || !GEngine || radius <= 0.f )
		{
			return;
		}

		UWorld* world = GEngine->GetWorldFromContextObject(
			worldContextObject, EGetWorldErrorMode::LogAndReturnNull );
		if ( !world )
		{
			return;
		}

		constexpr int32 cSegments = 24;
		constexpr float cThickness = 1.5f;
		constexpr bool bPersistent = false;
		constexpr uint8 cDepthPriority = 0;

		DrawDebugSphere(
			world, center, radius, cSegments, color,
			bPersistent, FMath::Max( duration, 0.f ), cDepthPriority, cThickness );
#endif
	}
}
