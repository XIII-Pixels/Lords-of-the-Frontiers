#include "Camera/CameraZoomUtils.h"

#include "Camera/CameraComponent.h"
#include "Camera/StrategyCamera.h"

#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

namespace CameraZoomUtils
{
	float GetCameraZoomAlpha( const UObject* worldContextObject )
	{
		if ( !worldContextObject )
		{
			return 0.5f;
		}

		UWorld* world = worldContextObject->GetWorld();
		if ( !world )
		{
			return 0.5f;
		}

		APawn* playerPawn = UGameplayStatics::GetPlayerPawn( world, 0 );
		const AStrategyCamera* strategyCam = Cast<AStrategyCamera>( playerPawn );
		if ( !strategyCam || !strategyCam->Camera )
		{
			return 0.5f;
		}

		float currentZoom = 0.0f;
		if ( strategyCam->Camera->ProjectionMode == ECameraProjectionMode::Orthographic )
		{
			currentZoom = strategyCam->Camera->OrthoWidth;
		}
		else if ( strategyCam->SpringArm )
		{
			currentZoom = strategyCam->SpringArm->TargetArmLength;
		}
		else
		{
			return 0.5f;
		}

		const float minZoom = FMath::Max( 1.0f, strategyCam->MinZoom() );
		const float maxZoom = FMath::Max( minZoom + 1.0f, strategyCam->MaxZoom() );
		const float safeZoom = FMath::Clamp( currentZoom, minZoom, maxZoom );

		const float invCurrent = 1.0f / safeZoom;
		const float invMin = 1.0f / minZoom;
		const float invMax = 1.0f / maxZoom;

		const float closeness = ( invCurrent - invMax ) / ( invMin - invMax );

		return FMath::Clamp( 1.0f - closeness, 0.0f, 1.0f );
	}

	float ScaleFromZoomAlpha( float zoomAlpha, float minScale, float maxScale )
	{
		const float clamped = FMath::Clamp( zoomAlpha, 0.0f, 1.0f );
		return FMath::Lerp( maxScale, minScale, clamped );
	}
}
