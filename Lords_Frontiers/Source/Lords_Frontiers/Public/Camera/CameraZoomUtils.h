#pragma once

#include "CoreMinimal.h"

class UObject;

namespace CameraZoomUtils
{
	float GetCameraZoomAlpha( const UObject* worldContextObject );

	float ScaleFromZoomAlpha( float zoomAlpha, float minScale, float maxScale );
}
